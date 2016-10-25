Vagrant.configure(2) do |config|

  config.vm.define "deb_build" do |deb_build|
    deb_build.vm.box = "debian/contrib-jessie64"
    deb_build.vm.provider "virtualbox" do |v|
      v.memory = 2048
    end
    deb_build.vm.synced_folder ".", "/mk"
    deb_build.vm.provision "shell", inline: <<-SHELL
      cd /mk
      ./build/docker/script/run gcc-jessie depend start_over
      echo "Now run:"
      echo "1. vagrant ssh"
      echo "2. cd /mk"
      echo "3. ./build/vagrant/jessie64"
      echo "4. sudo make install"
    SHELL
  end

  config.vm.define "deb_package" do |deb_package|
    deb_package.vm.box = "debian/contrib-jessie64"
    deb_package.vm.provider "virtualbox" do |v|
      v.memory = 2048
    end
    deb_package.vm.provision "shell", inline: <<-SHELL
      sudo apt-get install -y autotools-dev debhelper devscripts lintian       \
        autoconf automake libtool libssl-dev libevent-dev libgeoip-dev git
      git clone --single-branch --depth 1 --branch debian                      \
        https://github.com/measurement-kit/measurement-kit
      cd measurement-kit
      ./autogen.sh
      # See <http://askubuntu.com/a/675211>
      dpkg-buildpackage -b -rfakeroot -us -uc
    SHELL
  end

end
