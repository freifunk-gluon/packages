This module adds a respondd airtime usage statistics provider.
The format is the following:

```json
{
  "statistics": {
    "wireless": [
      {
        "frequency": 5220,
        "active": 366561161,
        "busy": 46496566,
        "rx": 808415,
        "tx": 41711344,
        "noise": 162,
        "phy": 1
      },
      {
        "frequency": 2437,
        "active": 366649704,
        "busy": 205221222,
        "rx": 108121446,
        "tx": 85453679,
        "noise": 161,
        "phy": 0
      }
    ]
  }
}
```

The numbers `active`, `busy`, `rx` and `tx` are times in milliseconds, where
`busy`, `rx` and `tx` have to be interpreted by taking the quotient with
`active`.
`phy` is the index of the radio in the mac80211 subsystem.

The motivation for having a list with the frequency as a value in the objects
instead of having an object with the frequency as keys is that multiple wifi
devices might be present, in which case the same frequency can appear multiple
times (because the statistics are reported once for every phy).

The field `phy` is added to distinguish multiple radios in the same band.
It is not the key of mapped data due to backwards compatibility.
