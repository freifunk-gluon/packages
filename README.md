# Gluon Packages Feed

This repository contains OpenWrt package definitions and software components used by the [Gluon](https://github.com/freifunk-gluon/gluon) firmware framework.

## Usage

To use this feed in an OpenWrt build environment, add it to your `feeds.conf` or `feeds.conf.default`:

```text
src-git gluon_packages https://github.com/freifunk-gluon/packages.git
```

Then update and install the packages:

```sh
./scripts/feeds update gluon_packages
./scripts/feeds install -a -p gluon_packages
```

