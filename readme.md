## Cobia :: Virtual Machine Manager
Quickly whip up a Cloud OS virtual machine with qemu and libvirt.

### Dependencies
- qemu
- libvirt
- mkisofs (cdr-tools)
- libxml2
- libyaml
- libwww-curl-perl
- libyaml-tiny-perl
- osinfo-db-tools ( optional )

### Install
Default location is **/usr/local/bin**.
```bash
$ make
$ make install
$ make uninstall
$ make clean
```

### Global Configuration
Can be configured on **~/.config/cobia/cobia.conf**.
```vimwiki
- images  -> List of cloud image urls for download
- ssh_key -> Full path to your chosen ssh .pub key
- user    -> Name of  VM user that will be created on install
- vms_dir -> Directory to install virtual machines
```

### Usage

**Bootstrap domain configuration:**
```bash
$ cobia-cfg --interactive --apply
$ cobia-cfg --os=2 --os-name="debian13" --apply # Create Debian domain yaml file
```

```yaml
---
domain:
  boot: uefi
  cpu: '2'
  devices:
    disks:
        '0':
            size: '40'
            source: /debian13-1792325918/debian13-1792325918.qcow2
            type: hdd
        '1':
            source: /debian13-1792325918/cidata.iso
            type: cd
    graphics: spice
    net:
        model: virtio
        source: default
    video:
      driver: qemu
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

**List domains:**
```bash
$ cobia -l domains
```

**Show domain info:**
```bash
$ cobia -d info <domain name>
```

**Start domain:**
```bash
$ cobia -d start <domain name>
```

**Stop domain:**
```bash
$ cobia -d stop <domain name>
```

**Create ( define ) a domain:**
```bash
$ cobia-cfg --os=2 --os-name="debina13" --create # Like "--apply", but immediately creates the domain
$ cobia -d create /path/to/domain/file.yml
```
Note: The correct OS name can be found via: `os-query os` command.

**Delete ( undefine ) domain:**
```bash
$ cobia -d delete <domain name>
```

**Connect to console:**
```bash
$ cobia -d shell <domain name>
```

**List networks:**
```bash
$ cobia -l networks
```

**Show network info:**
```bash
$ cobia -n info <net name>
```

**Create network:**
Creating a network can be done via a network yaml configuration file.
```yaml
---
network:
  name: new
  forward: nat
  bridge: virbr123
  ip:
    address: 192.168.123.1
    netmask: 255.255.255.0
    dhcp:
      range:
        start: 192.168.123.2
        end: 192.168.123.254
```

```bash
$ cobia -n create /path/to/net/file.yml
```

**Start network:**
```bash
$ cobia -n start <net name>
```

**Stop network:**
```bash
$ cobia -n stop <net name>
```

**Delete network:**
```bash
$ cobia -n delete <net name>
```
