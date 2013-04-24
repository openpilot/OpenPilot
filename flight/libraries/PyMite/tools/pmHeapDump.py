#!/usr/bin/env python

# This file is Copyright 2009, 2010 Dean Hall.
#
# This file is part of the Python-on-a-Chip program.
# Python-on-a-Chip is free software: you can redistribute it and/or modify
# it under the terms of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1.
#
# Python-on-a-Chip is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# A copy of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1
# is seen in the file COPYING in this directory.

"""
PyMite Heap Dump
================

Parses a heap dump into human-readable format.

The heap dump file is created by inserting a calls to heap_dump()
inside heap_gcRun().  Using two calls, a before and after, is ideal.

The dump format is:

=========== ==========================================
NumBytes    Contents
=========== ==========================================
6           string: PMDUMP or PMUDMP depending on target endianess (little and big respectively)
2           uint16: pointer size
2           uint16: dump format version
2			uint16: bifield of pmfeatures enabled on target
4			uint32: HEAP_SIZE
p			pointer to heap start
HEAP_SIZE   contents of heap (byte array)
4           uint16: NUM_ROOTS
NUM_ROOTS*p pointers to root objects
=========== ==========================================

The heap_dump() function names files incrementally starting from:

    pmheapdump00.bin
    pmheapdump01.bin
    ...
    pmheapdumpNN.bin
"""

## @file
#  @copybrief pmHeapDump

## @package pmHeapDump
#  @brief PyMite Heap Dump
#
#  See the source docstring for details.


import os, struct, sys, types, UserDict, collections

try:
    import cStringIO as StringIO
except:
    import StringIO


def _ellipse(string, length):
    """Truncates a string to a given size with ellipses
    """
    if len(string) < length:
        return string
    else:
        return string[0 : length-3] + '...'


def _dot_escape(string):
    return string.replace('<', '\<').replace('>', '\>')


def unpack_fp(fmt, fp, increment=True):
    """Unpacks a structure from a file, increment the position if set.
    """

    if increment:
        return struct.unpack(fmt, fp.read(struct.calcsize(fmt)))
    else:
        pos = fp.tell()
        ret = struct.unpack(fmt, fp.read(struct.calcsize(fmt)))
        fp.seek(pos)
        return ret


PmFieldInfo = collections.namedtuple('PmFieldInfo', 'name type mul')
PmBitfieldInfo = collections.namedtuple('PmFieldInfo', 'type fields mul')


class PmTypeInfo(object):
    """Model of an object type
    """

    def __init__(self, name, fmt):
        """Initializes a new object type from name and a format string
        The format consist of a list of field name, type and optionnal multiplicity
        	fmt = "fieldname1:fieldtype1[:multiplicity1],fieldname2:fieldtype2[:multiplicity2],..."
        where:
        	fieldname : a string naming a field...
            fieldtype : a type understand by python struct [c, b, B, ?, h, H, i, I, l, L, q, Q, f, d, s, p, P] or '.' for a bit
                P (pointer) is translated to the correct size based on the dump header information
            multiplicity : number of item for a list. multiplicity can be:
            	any positive integer
                a string naming a field already described
            Multiplicity can also specify:
                '*' to read as much as possible in the limit of the size object
                '<name' to read as long as memory offset is lower than then value of field name
        """

        self._parse_fmt(fmt)
        self.name = name


    def _parse_fmt(self, fmt):

        self.fields = []
        for f in map(lambda s: s.strip(), fmt.split(',')):
            if f == '':
                continue
            name, typ = f.split(':', 1)
            if ':' in typ:
                typ, mul = typ.split(':')
                try:
                    mul = int(mul)
                except:
                    pass
            else:
                mul = False

            self.fields.append(PmFieldInfo(name, typ, mul))

        # Bitfield support : concatenate bit-fields in a bitfield
        bitfield = None
        self._rawfields = []

        for i, f in enumerate(self.fields):

            if f.type != '.':
                self._rawfields.append(f)
                continue

            if bitfield == None:
                bitfield = []

            bitfield.append(f)

            # Next field is a bit in this bitfield
            if i + 1 < len(self.fields) and self.fields[i + 1].type == '.':
                continue

            # Last bit in field : sum the bitfield and add it to the fields list
            bitcount = sum([sf.mul for sf in bitfield])
            bytes = 1
            while (bytes * 8 < bitcount):
                bytes *= bytes
            typechr = [None, 'B', 'H', None, 'I', None, None, None, 'Q'][bytes]

            self._rawfields.append(PmBitfieldInfo(typechr, bitfield, False))
            bitfield = None


    def _calc_fieldmap(self, obj):
        """Computes fieldmap and format
        Returns a triple(format, : a struct format string
        		         fielmap, : a list of Pm(Bit)FieldInfo
                         remaining : number of field that cannot ne computed due to a missing value
    	                )
        """

        d = obj.data
        fmt = obj.heap.endianchr + "H" # Skip object descriptor
        fieldmap = [None]

        for i, f in enumerate(self._rawfields):
            typechr = (f.type == 'P') and obj.heap.ptrchr or f.type

            while (struct.calcsize(fmt) % struct.calcsize(typechr)!=0):
                fmt += 'x'

            if f.mul == False:
                fmt += typechr
                fieldmap.append(f)

            else:
                d[f.name] = []
                if isinstance(f.mul, int):
                    fmt += typechr * f.mul
                    fieldmap += [f] * f.mul
                elif f.mul == '*':
                    while struct.calcsize(fmt)<obj.size:
                        fmt += typechr
                        fieldmap.append(f)
                elif f.mul.startswith('<'):
                    if f.mul[1:] not in d:
                        return (fmt, fieldmap, len(self._rawfields)-i)

                    while struct.calcsize(fmt + typechr) + obj.addr \
                          + obj.heap.base < d[f.mul[1:]]:
                              # and struct.calcsize(fmt+typechr)<obj.size:
                        fmt += typechr
                        fieldmap += [f]
                else:
                    if f.mul not in d:
                        return (fmt, fieldmap, len(self._rawfields)-i)

                    fmt += typechr * d[f.mul]
                    fieldmap += [f] * d[f.mul]

        return (fmt, fieldmap, 0)


def PmObjectClass(dumpversion, features):

    class PmObject(UserDict.UserDict):
        """A model of an object.
        """

        PM_TYPES = (
            PmTypeInfo('NON', ""),
            PmTypeInfo("INT", "val:i"),
            PmTypeInfo("FLT", "val:f"),
            PmTypeInfo("STR", "len:H,"+
                       (features.USE_STRING_CACHE and "cache_next:P," or "") +
                       "val:B:len"),
            PmTypeInfo("TUP", "len:H,items:P:len"),
            PmTypeInfo("COB", "codeimg:P,names:P,consts:P,code:P"),
            PmTypeInfo("MOD", "co:P,attrs:P,globals:P," +
                       (features.HAVE_DEFAULTARGS and "defaultargs:P," or "") +
                       (features.HAVE_CLOSURES and "closure:P," or "")),
            PmTypeInfo("CLO", "attrs:P,bases:P"),
            PmTypeInfo("FXN", "co:P,attrs:P,globals:P," +
                       (features.HAVE_DEFAULTARGS and "defaultargs:P," or "") +
                       (features.HAVE_CLOSURES and "closure:P" or "")),
            PmTypeInfo("CLI", "class:P,attrs:P"),
            PmTypeInfo("CIM", "data:B:*"),
            PmTypeInfo("NIM", ""),
            PmTypeInfo("NOB", "argcount:B,funcidx:H"),
            PmTypeInfo("THR", "frame:P,interpctrl:I"),
            PmTypeInfo("x", ""),
            PmTypeInfo("BOL", "val:i"),
            PmTypeInfo("CIO", "data:B:*"),
            PmTypeInfo("MTH", "instance:P,func:P,attrs:P"),
            PmTypeInfo("LST", "len:H,sgl:P"),
            PmTypeInfo("DIC", "len:H,keys:P,vals:P"),
            PmTypeInfo("x", ""),
            PmTypeInfo("x", ""),
            PmTypeInfo("x", ""),
            PmTypeInfo("x", ""),
            PmTypeInfo("x", ""),
            PmTypeInfo("FRM", "back:P,func:P,memspace:B,ip:P,blockstack:P,"
                              "attrs:P,globals:P,sp:P,isImport:.," +
                       (features.HAVE_CLASSES and "isInit:.," or "") +
                       "locals:P:<sp"),
            PmTypeInfo("BLK", "sp:P,handler:P,type:B,next:P"),
            PmTypeInfo("SEG", "items:P:8,next:P"),
            PmTypeInfo("SGL", "rootseg:P,lastseg:P,length:H"),
            PmTypeInfo("SQI", "sequence:P,index:H"),
            PmTypeInfo("NFM", "back:P,func:P,stack:P,active:B,numlocals:B,"
                              "locals:P:8"),
            )

        FREE_TYPE = PmTypeInfo("FRE", "prev:P,next:P")


        def __init__(self, heap):
            """Initializes the object at the current file location
            """
            UserDict.UserDict.__init__(self)

            self.is_dotted = False
            self.is_dotrev =  False

            self.heap = heap
            self.fp = fp = self.heap.rawheap
            self.addr = self.fp.tell()

            od = unpack_fp(heap.endianchr + "H", fp, False)[0]
            self.mark = (' ','M')[(od & 0x4000) == 0x4000]
            self.free = (' ','F')[(od & 0x8000) == 0x8000]

            if self.free == 'F':
                self.size = (od & 0x3FFF) << 2
                self.objtype = self.FREE_TYPE

            else:
                self.size = (od & 0x01FF) << 2
                assert self.size > 0
                self.typeindex = (od >> 9) & 0x1f
                self.objtype = PmObject.PM_TYPES[self.typeindex]
                if self.objtype.name == 'x':
                    raise Exception("unknown object type", self.typeindex)

            self.type = self.objtype.name.lower()

            self.parse()

            self.fp.seek(self.addr + self.size)


        def parse(self,):
            """Parses data at the current file location
            """
            d = self.data

            fmt, fieldmap, remaining = self.objtype._calc_fieldmap(self)

            results = unpack_fp(fmt, self.fp, False)

            # Store data in obj
            for r, f in zip(results, fieldmap):
                if f == None:
                    continue
                elif isinstance(f, PmBitfieldInfo):
                    for bf in f.fields:
                        d[bf.name] = r & ((1 << bf.mul) - 1)
                        r = r >> bf.mul

                elif f.mul == False:
                    d[f.name] = r

                elif f.mul == 'lastv':
                    if r != 0:
                        d[f.name].append(r)
                    else:
                        break

                else:
                    d[f.name].append(r)

            if remaining:
                if not self.objtype._calc_fieldmap(self)[2] < remaining:
                    raise Exception("Cannot compute %s field length"
                                    % self.objtype._rawfields[-remaining].name)

                # Retry if there are any remaining field
                self.parse()


        def __str__(self,):

            d = self.data
            result = []
            result.append("%s %s %d %s%s: " % (
                hex(self.addr+self.heap.base),
                self.type,
                self.size,
                self.mark,
                self.free,))

            values = []
            for f in self.objtype.fields:
                typechr = (f.type in ['P', '.']) and "0x%x" or "%d"
                if f.mul == False:
                    values.append(("%s=" + typechr) % (f.name, d[f.name]))
                elif self.objtype.name == "STR" and f.name == "val":
                    values.append("val=%s"
                                  % _ellipse("".join(map(chr, d['val'])), 30))
                elif self.objtype.name == 'CIO' and f.name == 'data':
                    values.append("data=%s"
                                  % _ellipse(repr("".join(map(chr, d['data']))),
                                             30))
                else:
                    values.append(
                        "%s=[%s]"
                        % (f.name, ", ".join([typechr % v for v in d[f.name]])))
            result.append(", ".join(values))

            return "".join(result)


        def __repr__(self):
            return "<0x%x %s %d>" \
                   % (self.addr + self.heap.base,
                      self.objtype.name.lower(),
                      self.size)


        COLOR = ["aliceblue", "antiquewhite", "aqua", "aquamarine", "azure",
                 "beige", "bisque", "blanchedalmond", "blue",
                 "blueviolet", "brown", "burlywood", "cadetblue", "chartreuse",
                 "chocolate", "coral", "cornflowerblue", "cornsilk", "crimson",
                 "cyan", "darkblue", "darkcyan", "darkgoldenrod", "darkgray",
                 "darkgreen", "darkgrey", "darkkhaki", "darkmagenta",
                 "darkolivegreen", "darkorange", "darkorchid", "darkred",
                 "darksalmon", "darkseagreen", "darkslateblue", "darkslategray",
                 "darkslategrey", "darkturquoise", "darkviolet", "deeppink",
                 "deepskyblue", "dimgray", "dimgrey", "dodgerblue"]


        def dotstring(self):
            """A DOT representation of the object
            """

            if self.is_dotted:
                return "" #blurp %x' % (self.addr+self.heap.base)

            self.is_dotted =  True

            d = self.data

            #mark dic vals segment for upside-down display
            if self.type == 'dic' and d['len']>0:
                seg = self.heap.data[d['vals']].data['rootseg']
                while seg:
                    self.heap.data[seg].is_dotrev = True
                    seg = self.heap.data[seg].data['next']

            result = []

            result.append('"0x%x" [style=filled, fillcolor=%s, colorscheme=svg,'
                          ' label="%s"];'
                          % (self.addr+self.heap.base,
                             self.COLOR[getattr(self, 'typeindex', 0)],
                             self._dot_label()))

            if (self.type == "sgl" and d['rootseg'] in self.heap.data
                and d['rootseg'] != d['lastseg']):
                result.append("{ rank=same;")
                seg = d['rootseg']
                while seg in self.heap.data:
                    result.append(self.heap.data[seg].dotstring())
                    seg = self.heap.data[seg].data['next']
                result.append('}')
            return "\n".join(result)


        def _dot_label(self):
            """Label for the dot node
            """

            d = self.data
            label = []
            label.append('{')
            label.append(_dot_escape(repr(self)))

            values = []
            for f in self.objtype.fields:
                if f.type == 'P': continue
                if self.objtype.name == "STR" and f.name == "val" \
                   and d['val'] != None:
                    values.append("val=%s"
                        % _dot_escape(_ellipse("".join(map(chr, d['val'])), 20))
                        )
                elif self.objtype.name == "CIO" and f.name == "data":
                    values.append("data=%s"
                        % _dot_escape(_ellipse(repr(d['data']), 20)))
                else:
                    values.append("%s=%s"
                        % (_dot_escape(f.name), _dot_escape(str(d[f.name]))))
            if len(values):
                label.append("|{")
                label.append("|".join(values))
                label.append("}")

            pointers = []
            for f in self.objtype.fields:
                if f.type != 'P':
                    continue
                if f.name == "cache_next" and d[f.name] in self.heap.data:
                    pointers.append("%s=%s"%(f.name, hex(d[f.name])))
                else:
                    pointers.append("<%s> %s" % (f.name, f.name))
            if len(pointers):
                label.append('|{')
                label.append("|".join(pointers))
                label.append('}')
            label.append('}')

            return "".join(label)


        def dotedges(self):
            """Edges (pointers) leaving this object
            """

            d = self.data
            result = []

            for f in self.objtype.fields:
                if f.type != 'P':
                    continue
                if f.name == "cache_next":
                    continue
                if f.mul == False:
                    if d[f.name] == 0:
                        continue
                    result.append(self._dotedge(f.name, d[f.name]))
                else:
                    for i, m in enumerate(d[f.name]):
                        if m == 0:
                            continue
                        result.append(self._dotedge(f.name, m, str(i)))

            if self.type == "dic" and d['len'] > 0:
                i = 0
                sgls = tuple(map(lambda p: self.heap.data[d[p]],
                                 ("keys", "vals")))
                segs = tuple(map(lambda l: self.heap.data[l.data['rootseg']],
                                 sgls))
                iters = tuple(map(lambda s: iter(s.data['items']), segs))
                while i < d['len']:
                    try:
                        key, val = tuple(map(next, iters))
                        result.append('"0x%x" -> "0x%x" '
                                      '[style=dotted, weight=50];'%(key, val))
                        i += 1
                    except StopIteration:
                        segs = tuple(map(
                            lambda s: self.heap.data[s.data['next']] , segs))
                        iters = tuple(map(
                            lambda s: iter(s.data['items']), segs))

            return "\n".join(result)


        def _dotedge(self, name, value, label = None):

            style = []
            if label != None:
                style.append("label=%s" % label)
            if self.is_dotrev:
                style.append("dir=back")

            style = len(style) and (" [" + ", ".join(style) + "]") or ""

            if self.is_dotrev:
                return '"0x%x" -> "0x%x":%s%s;' \
                       % (value, self.addr+self.heap.base, name, style)
            else:
                return '"0x%x":%s -> "0x%x"%s;' \
                       % (self.addr+self.heap.base, name, value, style)

    return PmObject


class PmHeap(UserDict.UserDict):
    """A model of the heap.
    """

    FEATURES = ['USE_STRING_CACHE', 'HAVE_DEFAULTARGS', 'HAVE_CLOSURES',
                'HAVE_CLASSES']


    def __init__(self, fp):
        """Initializes the heap based on the given dump file.
        """
        UserDict.UserDict.__init__(self)

        self.is_parsed = False

        self._sense_fmt(fp)
        self.version, features, self.size, self.base = \
            unpack_fp(self.endianchr + "2HI" + self.ptrchr, fp)

        if self.version != 1:
            raise Exception('Dump version %d not supported' % self.version)

        self.features = \
            type("pmFeatures",
                 (object,),
                 dict(zip(self.FEATURES, [False] * len(self.FEATURES))))()
        f = 0
        while(features):
            setattr(self.features,
                    self.FEATURES[f],
                    features & 1 and True or False)
            f = f + 1
            features = features >> 1

        self.rawheap = StringIO.StringIO(fp.read(self.size))

        num_roots = unpack_fp("I", fp)[0]
        roots = {}
        (roots['None'],
         roots['False'],
         roots['True'],
         roots['Zero'],
         roots['One'],
         roots['NegOne'],
         roots['CodeStr'],
         roots['Builtins'],
         roots['NativeFrame'],
         roots['ThreadList']) = \
            unpack_fp(self.endianchr + (self.ptrchr * num_roots), fp)
        self.roots = roots
        self.PmObjectClass = PmObjectClass(self.version, self.features)

        fp.close()


    def _sense_fmt(self, fp):
        """Senses pmdump format (endianess, pointer size)
        depending on the first 8 bytes
        """

        magic = fp.read(6)

        if magic == "PMDUMP":
            self.endianess = "little"
            self.endianchr = '<'
        elif magic == "PMUDMP":
            self.endianess = "big"
            self.endianchr = '>'
        else:
            raise Exception("Not a PMDUMP format")

        self.ptrsize = unpack_fp(self.endianchr+"H", fp)[0]
        self.ptrchr = [None, 'B', 'H', None, 'I',
                       None, None, None, 'Q'][self.ptrsize]
        if self.ptrchr == None:
            raise Exception('invalid pointer size')


    def parse_heap(self,):
        """Parses the heap into a dict of key=address, value=object items
        """
        self.rawheap.seek(0)
        while self.rawheap.tell() < self.size:
            addr = self.rawheap.tell() + self.base
            self.data[addr] = self.PmObjectClass(self)
        self.is_parsed = True


    def __getitem__(self, indx):
        """Returns the object at the given address
        or the string of bytes defined by the slice.
        """
        # Return the object at the given address
        if type(indx) == types.IntType:
            if is_parsed:
                return self.data[indx]
            else:
                self.rawheap.seek(indx)
                return self.PmObjectClass(self)

        # Return the string of bytes defined by the slice
        elif type(indx) == types.SliceType:
            return self.rawheap[indx.start - self.base : indx.stop - self.base]

        else:
            assert False, "Bad type to heap[%s]" % type(indx)


    def __str__(self):

        d = self.data

        obj = filter(lambda o: o.type != "fre", self.data.values())
        free = filter(lambda o: o.type == "fre", self.data.values())

        result = []
        result.append("dump : version=%d, ptr=%dbytes, %s-endian, features=%s"
            % (self.version, self.ptrsize, self.endianess, self.features))
        result.append("roots : "
            + ", ".join(map(lambda kv: "%s=0x%x" % kv, self.roots.iteritems())))
        result.append("heap : size=%d, base=%x" % (self.size, self.base))
        result.append("summary : %d bytes in %d objects, %d free bytes" %
                      (sum([o.size for o in obj]),
                       len(obj),
                       sum([o.size for o in free])))

        for o in sorted(d.values(), key=lambda o: o.addr):
            result.append(str(o))

        result.append('')

        return "\n".join(result)


    def dotstring(self):
        """A DOT representation of the heap
        """

        d = self.data
        result = []
        result.append("digraph pmheapdump {")
        #result.append("concentrate=true;")

        result.append('{ rank=same;')
        for r, m in self.roots.iteritems():
            result.append("%s;" % r)
        result.append('}')

        result.append("{ node [shape=record];")
        for o in sorted(d.values(), key=lambda o: o.addr):
            result.append(o.dotstring())
        result.append("}")

        for r, m in self.roots.iteritems():
            result.append('%s -> "0x%x";' % (r, m))

        for o in sorted(d.values(), key=lambda o: o.addr):
            result.append(o.dotedges())

        result.append("}")

        result.append('')

        return "\n".join(filter(len, result))


def main():
    from optparse import OptionParser

    parser = OptionParser(usage="usage: %prog [options] [dumpfile [output]]")
    parser.add_option("-f", "--format",
                      dest="format", default='list', choices=['list', 'dot'],
                      help="output format: list or dot [default: %default]")

    (options, args) = parser.parse_args()

    if len(args) == 0:
        fp = open(os.path.join(os.path.curdir, "pmheapdump00.bin"), 'rb')
        out = sys.stdout
    elif len(args) == 1:
        fp = open(args[0], 'rb')
        out = sys.stdout
    elif len(args) == 2:
        fp = open(args[0], 'rb')
        out = open(args[1], 'w')
    else:
        print "too many arguments"
        parser.print_help()
        sys.exit()

    heap0 = PmHeap(fp)
    heap0.parse_heap()

    if options.format == 'list':
        out.write(str(heap0))
    elif options.format == 'dot':
        out.write(heap0.dotstring())


if __name__ == "__main__":
    main()
