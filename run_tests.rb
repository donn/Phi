#!/usr/bin/env ruby
require 'pathname'
require_relative './bsd_exits.rb'

`which iverilog`
if $?.exitstatus != 0
    puts "iverilog not found."
    exit BSDExits[:UNAVAILABLE]
end

test_folder = File.join(".", "Tests")
test_folder_pathname = Pathname.new(test_folder)

tests = Dir[File.join(test_folder, "**", "*.phi")]

relative_phic = File.join(".", "phic")
absolute_phic = File.expand_path(relative_phic)


output_format_str = "%-40s %-5s %-5s"
valid_phi = 0
valid_iverilog = 0

puts output_format_str % ["Test", "Phi", "SV"]
for test in tests
    test_pathname = Pathname.new(test)

    cwd = File.dirname(test)
    file = File.basename(test)
    test_name = test_pathname.relative_path_from(test_folder_pathname)
    file_sv = file + ".sv"

    `cd #{cwd}; '#{absolute_phic}' -o '#{file_sv}' '#{file}' 2>&1`
    phi_exit = $?.exitstatus    
    if phi_exit == 0
        valid_phi += 1
    end
    
    `cd #{cwd}; 'iverilog' '#{file_sv}'  2>&1 && rm -f a.out`
    iv_exit = $?.exitstatus
    if iv_exit == 0
        valid_iverilog += 1
    end
    puts output_format_str % [test_name, phi_exit == 0 ? '⭕' : '❌', iv_exit == 0 ? '⭕' : '❌']
end
puts "---"
puts "#{valid_phi}/#{tests.count} passed Phi."
puts "#{valid_iverilog}/#{tests.count} passed IcarusVerilog."

exit (valid_iverilog == tests.count) ? BSDExits[:OK] : BSDExits[:DATAERR]