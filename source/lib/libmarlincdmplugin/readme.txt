[ Deliverable of Marlin CDM ]
- Marlin CDM
  This module offers Marlin DRM functionality to end user with Android Multi Media Framework.
Following libraries will be delivered by Marlin CDM.
 - libmarlincdmplugin.so
   The library is provided as a part of the Marlin CDM package.
  - Marlin MediaDrm Plug-in
    This plug-in provides Marlin DRM transaction to application through Android MediaDrm framework.
  - Marlin MediaCrypto Plug-in
    This plug-in provides crypto transaction using Marlin DRM technology to application through Android MediaCrypto framework.
 - libmarlincdm.so
   This module controls all Marlin DRM usecases of Marlin MediaDrm plug-in & Marlin MediaCrypto plug-in.
 - libjsoncpp.so
   This module provides parsing and creation transaction to use json format.
 - libmarlinagenthandler.so
   This module provides interface to adapt Marlin core module, please refer to [ Adapting Marlin Agent Handler ].
- marlin drm test Application
  This application offers Marlin DRM function to user with Marlin CDM.
  The Marlin CDM is controlled through Android Multimedia service.

[ Environment ]
The integrator should prepare these environment, which is required by the Marlin CDM.
- AOSP
  The Marlin CDM needs Android OS 4.4 since it only conforms to Android MediaDrm plug-in & MediaCrypto plug-in interfaces.
 - For details about Android MediaDrm & MediaCrypto,
   please refer to http://developer.android.com/reference/android/media/MediaDrm.html and http://developer.android.com/reference/android/media/MediaCrypto.html
 - Marlin DRM service server
 - To work with marlin drm test Application,
   The integrator should prepare or choose the Marlin DRM service server which supports the system required by services.

[ Supported function ]
 - User Registration / User Deregistration
 - Acquire Marlin BB rights
 - Delete marlin BB rights
 - Load marlin BB rights
 - Decrypt MP4 Common encryption media
 - Decrypt MP4 IPMP media
 - Decrypt MP4 marlin Adaptive Streaming
 - Get Metadata from Marlin BB rights
 - Get Device Property
 To get more detail information, Refer to API comment in MarlinCDMInterface.h.
 Rest of other functionality described in MarlinCDMInterfce.h is not supported yet.

[ Adapting Marlin Agent Handler ]
To adapt and integrate Marin CDM and enable Marlin DRM services, the integrator should build following libraries as well.
 - Malin Agent Handler module (libmarlinagenthandler.so)
   This module provides interface to adapt Marlin core module.
   The integrator need a implementation to adapt Marlin core module, please refer to API list for Marlin CDM porting layer.

[ Build Instruction ]
please refer to build-instruction.txt

[ Interface information]
Please refer cdm/include/MarlinCdmInterface.h

