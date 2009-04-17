require 'jafar/kernel'
require 'jafar/jmath/jmath'
Jafar.register_module Jafar::Jmath

module Jafar
  module Jmath
    def Jmath.vec2ToArray(v)
      strs = print(v).split(/\(|\,|\)/)
      return [strs[1].to_f, strs[2].to_f]
    end
    def Jmath.vec3ToArray(v)
      strs = print(v).split(/\(|\,|\)/)
      return [strs[1].to_f, strs[2].to_f, strs[3].to_f]
    end
    def Jmath.vecToArray(v)
      strs = print(v).split(/\(|\,|\)/)
      a = []
      for i in 0...v.size
        a << strs[i+1].to_f
      end
      return a
    end
    def Jmath.arrayToVec(a)
      s = "("
      for i in 0...(a.size-1)
        s += "#{a[i]},"
      end
      s += "#{a[a.size-1]})"
      v = Jmath::Vec.new(a.size)
      setValue(v,s)
      return v
    end
    def Jmath.matToArray(m)
      a = []
      print(m).split(/\[|\]/)[2].split("),(").each() { |x|
        l = []
        x.split(/\(|\,|\)/).each() { |y|
            l << y.to_f unless(y == "" or y == "\n")
        }
        a << l
      }
      return a
    end
    def Jmath.arrayToMat(a)
      s = "(("
      for i in 0...a.size
        s += "),(" unless(i == 0)
        for j in 0...a[i].size
          s += "," unless(j == 0)
          s+= "#{a[i][j]}"
        end
      end
      s+= "))"
      m = Jmath::Mat.new(a.size, a[0].size)
      setValue(m,s)
      return m
    end
  end
end
