#!/usr/bin/perl

package VMYaml;
use strict;
use warnings;

use Cobia::VMConfig;
use YAML::PP;
use YAML::PP::Common qw(
    PRESERVE_ORDER
    PRESERVE_SCALAR_STYLE
);

our $vm_yaml = YAML::PP->new( preserve => PRESERVE_ORDER );


sub write_vm_yaml {
    $VMConfig::domain_settings{cd} = "$VMConfig::vm_env/$VMConfig::domain_settings{cd}";
    $VMConfig::domain_settings{disk} = "$VMConfig::vm_env/$VMConfig::domain_settings{name}.qcow2";
    $VMConfig::domain_settings{vm_dir} = "$VMConfig::vm_env";

    tie my %firm_keys_yaml, 'YAML::PP::Preserve::Hash';
    %firm_keys_yaml = (
        feature => undef,
        '@name' => "enrolled-keys",
        '@enabled' => "no",
    );
    
    tie my %firm_secboot_yaml, 'YAML::PP::Preserve::Hash';
    %firm_secboot_yaml = (
       feature => undef,
       '@name' => "secure-boot",
       '@enabled' => "no",
    );
    
    tie my %os_yaml, 'YAML::PP::Preserve::Hash';
    %os_yaml = (
        '@firmware' => "efi",
    
        type => {
            '@arch'         => $VMConfig::domain_settings{arch},
            '@machine'      => $VMConfig::domain_settings{machine},
            '#type'         => "hvm",
        },
    
        boot => {
            '@dev'          => $VMConfig::domain_settings{bootdev},
        },
    
        firmware => [
            \%firm_keys_yaml,
            \%firm_secboot_yaml,
        ],
    
        loader => {
            '@readonly'     => "yes",
            '@type'         => "pflash",
            '@format'       => "qcow2",
            '#loader'       => "/usr/share/edk2/OvmfX64/OVMF_CODE_4M.qcow2",
        },
    
        nvram => {
            '@template'         => "/usr/share/edk2/OvmfX64/OVMF_VARS_4M.qcow2",
            '@templateFormat'   => "qcow2",
            '@format'           => "qcow2",
            '#nvram'            => "/var/lib/libvirt/qemu/nvram/" . $VMConfig::domain_settings{name} . "_VARS.qcow2",
        },
    );
    
    tie my %feats_yaml, 'YAML::PP::Preserve::Hash';
    %feats_yaml = (
        acpi => "",
        apic => "",
    
        vmport => {
            '@state' => "off",
        },
    
        smm => {
            '@state' => "on",
        },
    );
    
    tie my %memory_yaml, 'YAML::PP::Preserve::Hash';
    %memory_yaml = (
        '@unit' => "MiB",
        '#memory' => $VMConfig::domain_settings{ram},
    );
    
    tie my %vcpu_yaml, 'YAML::PP::Preserve::Hash';
    %vcpu_yaml = (
        '@placement' => "static",
        '#vcpu' => $VMConfig::domain_settings{cpu},
    );
    
    tie my %disk_yaml, 'YAML::PP::Preserve::Hash';
    %disk_yaml = (
        disk => undef,
        '@type' => "file",
        '@device' => "disk",
    
        driver => {
            '@name' => "qemu",
            '@type' => "qcow2",
        },
    
        source => {
            '@file' => $VMConfig::domain_settings{disk},
        },
    
        target => {
            '@dev'  => "vda",
            '@bus'  => "virtio",
        },
    
    );
    
    tie my %cd_yaml, 'YAML::PP::Preserve::Hash';
    %cd_yaml = (
        disk => undef,
        '@type' => "file",
        '@device' => "cdrom",
    
        driver => {
            '@name' => "qemu",
            '@type' => "raw",
        },
    
        source => {
            '@file' => $VMConfig::domain_settings{cd},
        },
    
        target => {
            '@dev'  => "sda",
            '@bus'  => "scsi",
        },
    
    );
    
    tie my %scsi_ctrl_yaml, 'YAML::PP::Preserve::Hash';
    %scsi_ctrl_yaml = (
        controller => undef,
        '@type' => "scsi",
        '@model' => "virtio-scsi",
    );
    
    tie my %usb_ctrl_yaml, 'YAML::PP::Preserve::Hash';
    %usb_ctrl_yaml = (
        controller => undef,
        '@type' => "usb",
        '@model' => "qemu-xhci",
    );
    
    tie my %sata_ctrl_yaml, 'YAML::PP::Preserve::Hash';
    %sata_ctrl_yaml = (
        controller => undef,
        '@type' => "sata",
    );
    
    tie my %graphs_yaml, 'YAML::PP::Preserve::Hash';
    %graphs_yaml = (
        graphics => undef,
        '@type' => $VMConfig::domain_settings{graphics},
        '@autoport' => "yes",
        listen => {
            '@type' => "address",
        },
    );
    
    tie my %serial_yaml, 'YAML::PP::Preserve::Hash';
    %serial_yaml = (
        serial => undef,
        '@type' => "pty",
    
        target => {
            '@type' => "isa-serial",
    
            model => {
                '@name' => "isa-serial",
            },
        },
    );
    
    tie my %console_yaml, 'YAML::PP::Preserve::Hash';
    %console_yaml = (
        console => undef,
        '@type' => "pty",
    
        target => {
            '@type' => "serial",
            '@port' => 0,
        },
    );
    
    tie my %network_yaml, 'YAML::PP::Preserve::Hash';
    %network_yaml = (
        interface => undef,
        '@type' => "network",
    
        source => {
            '@network' => $VMConfig::domain_settings{net_source},
        },
    
        model => {
            '@type' => $VMConfig::domain_settings{net_model},
        }
    );
    
    tie my %video_yaml, 'YAML::PP::Preserve::Hash';
    %video_yaml = (
        video => undef,
        driver => {
            '@name' => $VMConfig::domain_settings{video_driver},
        },
    
        model => {
            '@type' => $VMConfig::domain_settings{video_model},
            '@heads' => 1,
            '@primary' => "yes",
        },
    );
    
    tie my %domain_yaml, 'YAML::PP::Preserve::Hash';
    %domain_yaml = (
        '@type'     => "kvm",
        name        => $VMConfig::domain_settings{name},
        os          => \%os_yaml,
        features    => \%feats_yaml,
        memory      => \%memory_yaml,
        vcpu        => \%vcpu_yaml,
    
        devices     => [
            {emulator => "/usr/bin/qemu-system-x86_64"},
            \%disk_yaml,
            \%cd_yaml,
            \%scsi_ctrl_yaml,
            \%usb_ctrl_yaml,
            \%sata_ctrl_yaml,
            \%graphs_yaml,
            \%network_yaml,
            \%serial_yaml,
            \%console_yaml,
            \%video_yaml,
        ],
    
        ssh_key     => $VMConfig::domain_settings{ssh_key},
        user        => $VMConfig::domain_settings{user},
        vm_dir      => $VMConfig::domain_settings{vm_dir}
    );

    tie my %vm, 'YAML::PP::Preserve::Hash';
    %vm = ( domain => \%domain_yaml );

    my $filename = "$VMConfig::domain_settings{name}".".yml";
    $vm_yaml->dump_file($filename, \%vm);

    open my $f_yaml, "+<", $filename or die "$!";
    my @lines = <$f_yaml>;
    s/: null$/:/ for @lines;
    seek($f_yaml, 0, 0);
    print($f_yaml @lines);
    truncate($f_yaml, tell $f_yaml);
    close($f_yaml);
}
