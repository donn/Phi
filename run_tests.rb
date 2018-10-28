#!/usr/bin/env ruby

Dir["Examples/*.phi"].map do |item|
    puts "Testing #{item}..."
    testable = item
    testable += '/*' if File.directory?(item);
    `./phi #{item}`
    if $? != 0
        puts "Regression detected at test #{item}."
    else
        puts "Test successful."
    end
    puts
end