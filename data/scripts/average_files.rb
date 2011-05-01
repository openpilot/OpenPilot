#!/usr/bin/ruby

# This script averages the columns of multiple files
#
# Author: croussil


filepattern=ARGV[0]
filenummin=Integer(ARGV[1])
filenummax=Integer(ARGV[2])
filename_output=ARGV[3]
n = filenummax-filenummin+1

# open all files
file = Array.new(n-1)
for i in (0..n-1)
	file[i] = File.open(sprintf(filepattern, i+filenummin))
end
file_output = File.open(filename_output,"w")

while (line = file[0].gets)
	if line[0] == 35 or line[0] == 0 or line[0] == 10 then next end
	sum = line.split(' ').map{|s| s.to_f}
	
	for i in (1..n-1)
		while (line = file[i].gets)
			if line[0] == 35 or line[0] == 0 or line[0] == 10 then next end
			vec = line.split(' ').map{|s| s.to_f}
			(0..sum.length-1).each { |j| sum[j] += vec[j] }
			break
		end
	end
	
	(0..sum.length-1).each { |j| sum[j] /= n }
	file_output.puts(sum.join(" "))
end


for i in (0..n-1)
	file[i].close
end
file_output.close

