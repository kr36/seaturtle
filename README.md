# Seaturtle : Krypton Core Library

WARNING: Seaturtle is still beta software. Exercise caution while using.

Seaturtle is the core component of the Krypton web browser. It supplements the [Chromium Content module](http://www.chromium.org/developers/content-module) with configurable privacy and anonymity enhancements. Currently, libseaturtle only targets Android platforms.

The goal of this project is to provide the building blocks for a cross platform browser with easily configurable privacy features comparable to the [Tor Browser](https://www.torproject.org/projects/torbrowser/design/).

Krypton Anonymous on Play Store:

*[![Play Store Badge](https://developer.android.com/images/brand/en_app_rgb_wo_60.png)](https://play.google.com/store/apps/details?id=co.kr36.krypton.x)*

Krypton Premium on Play Store:

*[![Play Store Badge](https://developer.android.com/images/brand/en_app_rgb_wo_60.png)](https://play.google.com/store/apps/details?id=co.kr36.krypton.r)*

## Building

1. Follow the Chromium [directions](https://www.chromium.org/developers/how-tos/get-the-code) for syncing to a release branch. Seaturtle currently builds for the release **38.0.2125.102**. You may also want to get familiar with the [Android build instructions](https://code.google.com/p/chromium/wiki/AndroidBuildInstructions). If you have successfully built a content shell you are probably ready to proceed.

2. Extract seaturtle's code into the chromium source tree (src), under a top level directory named "seaturtle".

3. Apply the diffs in the file: chromium.diff

4. From the src directory:

```source ./build/android/envsetup.sh && ninja -C out/Release seaturtlegyp_apk```

## Relevant Reading

[Tor Browser Design](https://www.torproject.org/projects/torbrowser/design/).

[Technical analysis of client identification mechanisms](https://www.chromium.org/Home/chromium-security/client-identification-mechanisms)

[Tor Wiki - Important Google Chrome Bugs](https://trac.torproject.org/projects/tor/wiki/doc/ImportantGoogleChromeBugs)

## License

Copyright 2014 Kr36 LLC. All rights reserved.
Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
