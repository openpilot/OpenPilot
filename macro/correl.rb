require 'jafar/kernel'
require 'jafar/correl/correl'
Jafar.register_module Jafar::Correl


module Jafar
	module Correl

		# deprecated, no way to call ExplorerZncc.exploreTranslation, the function is moved in c++ in test_suite
		def Correl.trackPoint( filePrefix, fileNBegin, fileNEnd, fileNDigits, fileSuffix, x, y, winHalfSize, searchHalfW, searchHalfH )
			logfile = File.open("track.dat", "w")
			logfile.puts("# Correl.trackPoint")
			
			currentX = Kernel::create_float(x)
			currentY = Kernel::create_float(y)

			im1 = Image::Image.new
			im2 = Image::Image.new
			
			for i in fileNBegin..fileNEnd
				nFormat = "%0#{fileNDigits}d"
				imgName = "#{filePrefix}#{format(nFormat,i)}#{fileSuffix}"
				puts imgName
				
				if (i != fileNBegin)
					im1 = im2
					im1.set_roi(Image::cvRect(x-winHalfSize,y-winHalfSize,2*winHalfSize+1,2*winHalfSize+1))
				end
				im2 = Image::Image.new
				im2.load(imgName)
				
				if (i != fileNBegin)
					Correl::ExplorerZncc.exploreTranslation(im1, im2, x-searchHalfW, x+searchHalfW, 1, y-searchHalfH, y+searchHalfH, 1, currentX, currentY)
					x = currentX.to_i
					y = currentY.to_i
				end
				
				logfile.puts("#{i}	#{currentX}	#{currentY}")
				GC.start
			end
			
			
			logfile.close()
		end
		
		def Correl.correlFiles(file1, iX1, iY1, file2, iX2, iY2, xBefore, xAfter, xStep)
			
			f1 = File.open(file1)
			f2 = File.open(file2)

			# comment
			f1.gets
			f2.gets
			# we'll need it
			f1.gets
			f2.gets
			
			# alignment
			line1 = f1.gets
			line2 = f2.gets
			vec1 = line1.split(' ')
			vec2 = line2.split(' ')
			
			
			
			nsteps = (xAfter-xBefore)/xStep
			nsteps = nsteps.to_i
			for i in 0..nsteps
			
				while( line = f1.gets )
					
					data1 = line.split(',')
					x = data1[iX1].to_f
				end
			end
			
		end
		
		
	end
end


