MRuby::Gem::Specification.new("mruby-ckb") do |spec|
  spec.license = "Unlicensed"
  spec.author  = "Nervos Core Dev <dev@nervos.org>"
  spec.summary = "CKB related functions for mruby"
  spec.cc.flags << "-I../ckb-c-stdlib"
end
