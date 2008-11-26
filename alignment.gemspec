Gem::Specification.new do |s|
  s.name = %q{alignment}
  s.version = "0.1.0"
  s.authors = ["Marius L. J\303\270hndal"]
  s.date = %q{2008-11-26}
  s.description = %q{Alignment functions for corpus linguistics.}
  s.email = %q{mariuslj (at) ifi [dot] uio (dot) no}
  s.extensions = ["ext/gale_church/extconf.rb"]
  s.extra_rdoc_files = ["README.rdoc", "lib/alignment.rb"]
  s.files = ["README.rdoc", "Rakefile", "test/test_alignment.rb", "CHANGELOG", "ext/gale_church/gale_church.c", "ext/gale_church/extconf.rb", "lib/alignment.rb", "Manifest", "alignment.gemspec"]
  s.has_rdoc = true
  s.homepage = %q{http://github.com/mlj/alignment}
  s.rdoc_options = ["--line-numbers", "--inline-source", "--title", "Alignment", "--main", "README.rdoc"]
  s.summary = %q{Alignment functions for corpus linguistics.}
  s.test_files = ["test/test_alignment.rb"]
end
