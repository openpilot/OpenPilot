# Introduction

This is intended to be a node.js implementation of the uavtalk protocol.

The original cut was heavily copied from the node-uavtalk-proxy library from grantmd.  https://github.com/grantmd/node-uavtalk-proxy  But the latest version does not rely on that code base at all, but rather uses the uavobject generator from the base OpenPilot program to generate json descriptions of each object that will parse the resulting uavtalk packets from that definition.

The goal of this is to be able to control a flight controller over the uavtalk protocol.  My particular use case uses a Raspberry PI riding on the back of a quadcopter running the cc3d flight controller.

John Aughey - jha@aughey.com
https://github.com/aughey/nodeuavtalk
https://github.com/aughey/OpenPilot

# Prerequisites
Please install the underscore & bufferpack libraries
$ npm install -g underscore
$ npm install -g bufferpack
$ npm install -g serialport
$ npm install -g express
$ npm install -g socket.io

Set your NODE_PATH accordingly
$ export NODE_PATH="$(npm root -g)"
