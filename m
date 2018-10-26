#!/usr/bin/env ruby
require 'etc'

threads = Etc.nprocessors
exec('make', "-j#{thread}")