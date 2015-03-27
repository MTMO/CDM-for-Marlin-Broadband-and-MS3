Marlin Content Decryption Module
=============================

## Description
Marlin Content Decryption Module provides a way to acquire right and decrypt for Marlin protected content.

## Supported platform
 * Android OS
 * Firefox OS

## Supported functionality
 * User Registration / User Deregistration
 * Acquire Marlin BB rights
 * Delete Marlin BB rights
 * Load Marlin BB rights
 * MS3 SAS acquisition
 * Decrypt MPEG4 ISOBFF Common encryption media
 * Decrypt MPEG4 IPMP extension media
 * Get Metadata from Marlin BB rights
 * Get Device Property
To get detail information to enable listed functions, Refer to API document.
Rest of other functionality is not supported.

## Content support
  * Containers: MPEG4
  * Video Codec: H264.
  * Audio Codec: AAC.
  * Encryption scheme: ISO Based File Format Common Encryption for Marlin, MPEG4 IPMP extension for Marlin
  
## Developer Guide

### Deliverable of Marlin Content Decryption Module
#### Libraries
This offers Marlin functionality to client application with Android Multi Media Framework.
Following libraries will be built in this project.
 * **libmarlincdmplugin.so**
  - Marlin MediaDrm Plug-in  
    This plug-in provides rights transaction to application through Android MediaDrm framework.
  - Marlin MediaCrypto Plug-in  
    This plug-in provides crypto transaction for application to decrypt Marlin protected media through Android MediaCrypto framework.
 * **libmarlincdm.so**  
   This module controls all Marlin DRM usecases of Marlin MediaDrm plug-in and Marlin MediaCrypto plug-in.
 * **libjsoncpp.so**  
   This module provides parsing and creation transaction to use json format.
 * **libmarlinagenthandler.so**  
   This module provides interface to adapt Marlin core module. To build this library for your usage, please refer to **Adapting Marlin Agent Handler**.

### Environment
You should prepare these environment, which is required by the Marlin CDM.
 * Android Open Source or Mozilla B2G  
  The library needs to be run on Android OS 4.4 or later or latest Firefox OS.
 * Marlin DRM server  
  To work with Marlin aware Application, you should prepare or choose the encounter server.

### Adapting Marlin Agent Handler
To use the library and enable Marlin DRM services, you should implement the adaptation layer for your proprietary Marlin Agent and then build following library.
 * Malin Agent Handler module (libmarlinagenthandler.so)
This module provides interface for the library to adapt Marlin core agent.
For your adaptation, please refer the documentation.

### Build Instruction
To prepare your device to enable Marlin service with the library, please refer *build-instruction.txt*

### Test Application for Android environment
The test application code contain a Eclipse project for Android Test Application.
* Install Android SDK and update to use latest version
* Install Eclipse and setup with Android SDK
* Select to import the project on Eclipse
* Check *usage-instruction.txt* to run

### Test Application for Firefox OS environment
The test application code should be built on Firefox WebIDE.
* Install Firefox Developer Edition
* Select the packaged directory to open the project
