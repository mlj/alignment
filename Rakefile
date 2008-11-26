require 'rubygems'
require 'rake'
 
begin
  require 'echoe'
 
  Echoe.new('alignment', '0.1.0') do |p|
    p.summary = "Alignment functions for corpus linguistics."
    p.author = 'Marius L. Jøhndal'
    p.email = "mariuslj (at) ifi [dot] uio (dot) no"
    p.url = "http://github.com/mlj/alignment"
    p.ignore_pattern = ["*.gemspec"]
    p.rdoc_pattern = ["README.rdoc", "lib/*.rb"]
#    p.rubyforge_name = "alignment"
  end
 
rescue LoadError => boom
  puts "You are missing a dependency required for meta-operations on this gem."
  puts "#{boom.to_s.capitalize}."
end
 
Dir["#{File.dirname(__FILE__)}/tasks/*.rake"].sort.each { |ext| load ext }
