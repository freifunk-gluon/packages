# autoupdater

The autoupdater is an automated firmware upgrade daemon for OpenWrt and Gluon. It periodically checks configured mirror servers for signed update manifests, validates ECDSA cryptographic signatures, calculates a probability-based rollout schedule based on release priority, downloads the matching image, verifies its SHA-256 checksum, and triggers `sysupgrade`.

## Usage

```
Usage: autoupdater [options] [<mirror> ...]

Possible options are:
  -b, --branch BRANCH   Override the branch given in the configuration.
  -f, --force           Always upgrade to a new version, ignoring its priority
                        and whether the autoupdater is enabled.
  -h, --help            Show help text.
  -n, --no-action       Download and validate the manifest and firmware image,
                        but do not flash.
  --fallback            Upgrade if and only if the upgrade timespan of the new
                        version has passed for at least 24 hours.
  --force-version       Skip version check to allow downgrades.
  <mirror> ...          Override the mirror URLs given in the configuration. If
                        specified, these are not shuffled.
```

## Configuration

The autoupdater is configured via UCI in `/etc/config/autoupdater`:

```
config autoupdater 'settings'
	option enabled '1'
	option branch 'stable'
	# option version_file '/lib/gluon/release'

config branch 'stable'
	option name 'stable'
	list mirror 'http://[fdef:ffc0:3dd7::8]/~freifunk/firmware/autoupdate'
	option good_signatures '1'
	list pubkey 'beea7da92ed0c19563b6c259162b4cb471aa2fdf9d3939d05fea2cf498ea7642'
```

### Options

* `settings.enabled`: Enables (`1`) or disables (`0`) automated background updates.
* `settings.branch`: Default branch to follow when checking for updates.
* `settings.version_file`: Path to the file containing current firmware version (default: `/lib/gluon/release`).
* `branch.<name>.mirror`: List of HTTP/HTTPS mirrors serving firmware manifests and images.
* `branch.<name>.good_signatures`: Minimum number of valid ECDSA signatures required to accept a manifest.
* `branch.<name>.pubkey`: List of authorized ECDSA public keys.

## Manifest Format

The update manifest (`manifest`) contains metadata, image checksums, and cryptographic signatures:

```
BRANCH=stable
DATE=2026-09-01 12:00:00+00:00
PRIORITY=7

# model               ver sha256sum                                                        size    filename
tp-link-tl-wdr4300-v1 0.4 0ce0fb6a79802ba98c933ac3ae7757fdf2f62b32641fb6c5efc09211b9082c46 3735556 gluon-ffhl-0.4-tp-link-tl-wdr4300-v1-sysupgrade.bin

# after three dashes follow the ecdsa signatures of everything above the dashes
---
49030b7b394e0bd204e0faf17f2d2b2756b503c9d682b135deea42b34a09010bff139cbf7513be3f9f8aae126b7f6ff3a7bfe862a798eae9b005d75abbba770a
```

* `BRANCH`: Target branch name matching the configuration.
* `DATE`: Publication timestamp used as the start point for rollout probability calculation.
* `PRIORITY`: Number of days across which the update probability increases to 100%.
* `---`: Delimiter separating the signed content from the hex-encoded ECDSA signatures.

## Hook Directories

Custom scripts can hook into different stages of the update lifecycle:

* `/usr/lib/autoupdater/download.d/`: Executed after manifest verification, before the image download begins.
* `/usr/lib/autoupdater/abort.d/`: Executed if downloading the image fails, to revert changes made in `download.d`.
* `/usr/lib/autoupdater/upgrade.d/`: Executed immediately before `sysupgrade` is invoked.
