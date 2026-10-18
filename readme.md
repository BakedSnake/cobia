### Cobia :: Virtual Machine Manager
Quickly whip up a Cloud OS virtual machine with qemu and libvirt.

#### Dependencies
- qemu
- libvirt
- mkisofs (cdr-tools)
- libxml2
- libyaml
- libwww-curl-perl
- libyaml-tiny-perl

#### Install
Default location is **/usr/local/bin**.
```bash
$ make
$ make install
$ make uninstall
$ make clean
```

#### Configuration
Can be configured on **~/.config/cobia/cobia.conf**.
```vimwiki
- vms_dir -> Directory to install virtual machines
- user -> Name of  VM user that will be created on install
- ssh_key -> Full path to your chosen ssh .pub key
```

Default values for vm install.
```yaml
- name: Default Settings
  arch: "x86_64"
  boot: "uefi"
  cpu: "2"
  graphics: "spice"
  model: "virtio"
  net: "default"
  ram: "2048"
```

Note: These options assume you have a virtual network named default, and that it is enabled.

#### Usage

**Bootstrap domain configuration:**
```bash
$ cobia-cfg --interactive --apply
$ cobia-cfg --os=2 --os-name=debian13 --apply # Create Debian domain yaml file
```

```yaml
---
domain:
  boot: uefi
  cpu: '2'
  devices:
    cd: /debian13-1792325918/cidata.iso
    disk:
      size: '40'
      source: /debian13-1792325918/debian13-1792325918.qcow2
    graphics: spice
    net: default
    video:
      driver: qemu
      model: virtio
  model: virtio
  name: debian13-1792325918
  os:
    arch: x86_64
    bootdev: hd
    efi: 'false'
    features:
      secure_boot: no
    machine: q35
  ram: '2096'
  ssh_key: /keys/key/key_rsa.pub
  user: cg
  vm_dir: /debian13-1792325918
```

**Create a domain:**
```bash
$ cobia-cfg --os=2 --os-name="debina13" --create # Like "--apply", but immediately creates the domain
$ cobia -c /path/to/domain/file.yml
```
Note: The correct OS name can be found via: `os-query os` command.

**List domains:**
```bash
$ cobia -l domains
```

**Show domain info:**
```bash
$ cobia -i <vm name>
```

**List networks:**
```bash
$ cobia -l networks
```

