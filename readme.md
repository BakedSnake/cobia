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

**Configuring Host Network:**
```bash
$ cobia -c netconf <inferface name>
```

**Bootstrap domain configuration:**
```bash
$ cobia -c bootstrap --interactive --apply
$ cobia -c bootstrap --os=2 --name="debian13" --apply # Create Debian domain yaml file
```

```yaml
---
domain:
  '@type': kvm
  name: debian13
  os:
    '@firmware': efi
    type:
      '@machine': q35
      '@arch': x86_64
      '#type': hvm
    boot:
      '@dev': hd
    firmware:
    - feature:
      '@name': enrolled-keys
      '@enabled': no
    - feature:
      '@name': secure-boot
      '@enabled': no
    loader:
      '@readonly': yes
      '@format': qcow2
      '@type': pflash
      '#loader': /usr/share/edk2/OvmfX64/OVMF_CODE_4M.qcow2
    nvram:
      '#nvram': /var/lib/libvirt/qemu/nvram/debian13_VARS.qcow2
      '@template': /usr/share/edk2/OvmfX64/OVMF_VARS_4M.qcow2
      '@format': qcow2
      '@templateFormat': qcow2
  features:
    acpi: ''
    apic: ''
    vmport:
      '@state': off
    smm:
      '@state': on
  memory:
    '@unit': MiB
    '#memory': '2096'
  vcpu:
    '@placement': static
    '#vcpu': '2'
  devices:
  - emulator: /usr/bin/qemu-system-x86_64
  - disk:
    '@type': file
    '@device': disk
    driver:
      '@type': qcow2
      '@name': qemu
    source:
      '@file': debian13/debian13.qcow2
    target:
      '@dev': vda
      '@bus': virtio
  - disk:
    '@type': file
    '@device': cdrom
    driver:
      '@type': raw
      '@name': qemu
    source:
      '@file': debian13/cidata.iso
    target:
      '@bus': scsi
      '@dev': sda
  - controller:
    '@type': scsi
    '@model': virtio-scsi
  - controller:
    '@type': usb
    '@model': qemu-xhci
  - controller:
    '@type': sata
  - graphics:
    '@type': spice
    '@autoport': yes
    listen:
      '@type': address
  - interface:
    '@type': network
    source:
      '@network': default
    model:
      '@type': virtio
  - serial:
    '@type': pty
    target:
      model:
        '@name': isa-serial
      '@type': isa-serial
  - console:
    '@type': pty
    target:
      '@type': serial
      '@port': 0
  - video:
    driver:
      '@name': qemu
    model:
      '@type': virtio
      '@heads': 1
      '@primary': yes
  ssh_key: /keys/key/key_rsa.pub
  user: username
  vm_dir: /debian13
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
$ cobia -c bootstrap --os=2 --name="debian13" --os-name="debina13" --create # Like "--apply", but immediately creates the domain
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
  forward:
    '@mode': nat
    nat:
      port:
        '@start': 1024
        '@end': 65535
  bridge:
    '@name': virbr123
    '@stp': on
    '@delay': 0
  ip:
    '@address': 192.168.123.1
    '@netmask': 255.255.255.0
    dhcp:
      range:
        '@start': 192.168.123.2
        '@end': 192.168.123.254
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
