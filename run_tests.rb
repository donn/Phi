#!/usr/bin/env ruby

Dir["Examples/*"].map do |item|
    puts "Testing #{item}..."
    testable = item
    if File.directory?(item)
        testable += '/*'
    end
    `./phi #{item}`
    if $? != 0
        puts "Regression detected at test #{item}."
    else
        puts "Test successful."
    end
    puts
end