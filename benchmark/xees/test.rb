#!/usr/bin/env ruby

re = Regexp.new(ARGV[0])
while line = STDIN.gets
  puts line if line =~ re
end
