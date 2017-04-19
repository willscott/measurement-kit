Vagrant.configure(2) do |config|
  config.vm.synced_folder ".", "/mk"

  config.vm.define "yakkety" do |yakkety|
    yakkety.vm.box = "ubuntu/yakkety64"
    yakkety.vm.provider "virtualbox" do |v|
      v.memory = 2048
    end
  end

  config.vm.define "deb_package" do |deb_package|
    deb_package.vm.box = "debian/contrib-jessie64"
    deb_package.vm.provider "virtualbox" do |v|
      v.memory = 2048
    end
    deb_package.vm.provision "shell", inline: <<-SHELL
      sudo apt-get install -y autotools-dev debhelper devscripts lintian       \
        autoconf automake libtool libssl-dev libevent-dev libgeoip-dev git
      git clone --single-branch --depth 3 --branch debian                      \
        https://github.com/measurement-kit/measurement-kit
      cd measurement-kit
      ./autogen.sh
      # See <http://askubuntu.com/a/675211>
      dpkg-buildpackage -b -rfakeroot -us -uc
    SHELL
  end

end
