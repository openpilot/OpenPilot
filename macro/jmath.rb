require 'jafar/kernel'
require 'jafar/jmath/jmath'
Jafar.register_module Jafar::Jmath

module Jafar
  module Jmath
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
  end
end
