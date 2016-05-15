#!/usr/bin/ruby

require 'csv'

def c_string(s)
  s = s.gsub(/\n/, '\\\\n').gsub(/"/, '\\"')
  return '"' + s + '"'
end

puts '// AUTOGENERATED. DO NOT EDIT. INVOKE \'make\' TO REGENERATE.'
puts ''
puts '#include "quotes.h"'
puts ''
puts '#include <avr/pgmspace.h>'
puts ''

count = 0
CSV.new(STDIN).each do |row|
  puts "static char const SUBJECT_#{count}[] PROGMEM = #{c_string row[0]};"
  puts "static char const QUOTE_#{count}[] PROGMEM = #{c_string row[1]};"
  count += 1
end
puts ''

print 'char const *const SUBJECTS[] PROGMEM = { '
count.times do |i|
  print "SUBJECT_#{i}, "
end
puts '};'
puts ''

print 'char const *const QUOTES[] PROGMEM = { '
count.times do |i|
  print "QUOTE_#{i}, "
end
puts '};'
puts ''

puts "int const NUM_QUOTES = #{count};"