# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure(2) do |config|
  config.vm.box = "bento/debian-8.3"

  config.vm.provider :vmware_fusion do |v|
    if ARGV[0] == "up"
	  v.vmx["allowNested"] = "TRUE"
	  v.vmx["vhv.enable"] = "TRUE"
    end
  end
end
