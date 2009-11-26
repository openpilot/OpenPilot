require 'jafar/kernel'
require 'jafar/jmath/jmath'
Jafar.register_module Jafar::Jmath

module Jafar
  module Jmath
    def Jmath.vecToArray(v)
      strs = print(v).split(/\(|\,|\)/)
      a = []
      for i in 1...(strs.size-1)
        a << strs[i].to_f
      end
      return a
    end
    def Jmath.arrayToVec(a, type = Jmath::Vec)
      s = "("
      for i in 0...(a.size-1)
        s += "#{a[i]},"
      end
      s += "#{a[a.size-1]})"
      v = type.new(a.size)
      setValue(v,s)
      return v
    end
    def Jmath.arrayToVec3(a)
      return arrayToVec(a, Jmath::Vec3 )
    end
    def Jmath.arrayToVec2(a)
      return arrayToVec(a, Jmath::Vec2 )
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
