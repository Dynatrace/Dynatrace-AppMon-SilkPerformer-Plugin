# SilkPerformer Plugin

## Overview

![images_community/download/attachments/11567147/icon.png](images_community/download/attachments/11567147/icon.png) dynaTrace fully integrates with Micro Focus SilkPerformer (formely: Borland
SilkPerformer). It extends SilkPerformer's capabilities by pinpointing the root cause of performance, stability, and scalability problems in application code.

See the [dynaTrace blog](http://blog.dynatrace.com/2010/05/04/elevating-web-and-load-testing-with-microfocus-silkperformer-diagnostics-powered-by-dynatrace/) for a hands-on introduction incl. a step-
by-step guide and screenshots.

## Plugin Details

| Name | SilkPerformer Plugin
| :--- | :---
| Author | dynaTrace software
| Supported dynaTrace Version | >= 5.5
| Supported SilkPerformer Version | Tested with SilkPerformer 2007, 2008, 2008 R2, 2010, 2010 SP1, 2010 R2, 2011
| License | [dynaTrace Experimental Software License](dynaTraceES.txt)
| Support | [Limited Support](https://community.compuwareapm.com/community/display/DL/Support+Levels#SupportLevels-Supported)
| Download | [SilkPerformer Plugin for dynaTrace 5.6](silkperformer-diagnostics-plugin-5.6.0.5713.zip)  
|| [SilkPerformer Plugin for dynaTrace 5.5](silkperformer-diagnostics-plugin-5.5.0.5226.zip)  
|| [SilkPerformer Plugin for dynaTrace 5](attachments_100925539_1_silkperformer-diagnostics-plugin-5.0.0.3772.zip)  

### Installation

SilkPerformer 2010 SP1 ships with SilkPerformer Diagnostics Plugin v3.2.1 embedded.

Copy the plugin to the `<SilkPerformer_home>\Working\Plugins` directory and extract it. Start SilkPerformer, choose `"Settings"` -> `"System"` and scroll down in the left side bar until the plugin is
visible.

![images_community/download/attachments/11567147/silkperformerPlugin.png](images_community/download/attachments/11567147/silkperformerPlugin.png)

Figure: SilkPerformer Plugin installed in SilkPerfomer

Some SilkPerformer versions come with a dynaTrace Diagnostics Plugin installed. Please deactivate this plugin to use the SilkPerformer Plugin .

### How to use the SilkPerformer Plugin

The SilkPerformer Plugin offers the ability to record a session when a load test (even a try script) is started. This can be done by setting the option `"Automatic Session Recording"` in the Plugins
`"Attributes"` tab to true.

The `"Wait Timeout"` Attribute determines the time the SilkPerformer is waiting for the dynaTrace Client to be started. If the dynaTrace Client cannot be reached within this interval the load test
will start without dynaTrace session recording.

The `"System Profile"` Attribute determines the dynaTrace System Profile on which session recording should be started. If omitted, session recording will be started on the first System Profile on the
dynaTrace Server with an Agent connected.

The `"SilkPerformer Diagnostics Client Port"` Attribute determines the dynaTrace client REST interface port.

If the dynaTrace Client is connected to multiple dynaTrace servers the first connected server will be used.

If you change the configured server in dynaTrace Client SilkPerformer has to be restarted.

### Installation

Copy the Plugin to the `<SilkPerformer_home>\Working\Plugins` directory and extract it. Start SilkPerformer, choose `"Settings"` -> `"System"` and scroll down in the left side bar until the dynaTrace
Plugin is visible.

![images_community/download/attachments/11567147/silkperformer_settings_31.png](images_community/download/attachments/11567147/silkperformer_settings_31.png)

Figure: dynaTrace Plugin installed in SilkPerfomer

Most SilkPerformer versions come with a dynaTrace Diagnostics Plugin installed. Please deactivate this Plugin to use the dynaTrace Plugin with dynaTrace 3.0 / 3.1.

### How to use the SilkPerformer Plugin

The dynaTrace Plugin offers the ability to record a session when a load test (even a try script) is started. This can be done by setting the option `"Automatic Session Recording"` in the Plugins
`"Attribute"` tab to true.

The `"Wait Timeout"` Attribute determines the time the SilkPerformer is waiting for the dynaTrace Client to be started. If the dynaTrace Client cannot be reached within this interval the load test
will start without dynaTrace session recording.

The `"Application Session"` Attribute determines the dynaTrace System Profile on which session recording should be started. If omitted, session recording will be started on the first System Profile on
the dynaTrace Server with an Agent connected.

If the dynaTrace Client is connected to multiple dynaTrace Servers there may be problems with matching the correct System Profile. If the System Profile is available on multiple dynaTrace Servers, it
is possible that session recording is started on the wrong Server. If the `"profile name"` attribute is omitted, session recording may be started on any of the connected dynaTrace Server on the first
System Profile with an Agent connected.

## Patch for SilkPerformer JMX Integration

| Name | SilkPerformer JMX Browser Patch
| :--- | :---
| Description | This is the SilkPerformer JMX Browser Patch for dynaTrace 3.0.x and 3.1.
| Prerequisite | SilkPerformer 2006, 2006 R2, 2007, 2008, 2008 R2, 2009
| Version | 3.0.1
| Supported dynaTrace Version | >= 5.5
| Tested with | SilkPerformer 2007, 2008, 2008 R2, 2009
| Author | dynaTrace software
| License | [dynaTrace Experimental Software License](dynaTraceES.txt)
| Support | [Supported](https://community.compuwareapm.com/community/display/DL/Support+Levels#SupportLevels-Supported)
| Download | [SilkPerformer Patch](SilkPerformerPatch.zip)

All currently available SilkPerformer versions are missing the JMX integration data for dynaTrace 3.0. To be able to use dynaTrace 3.0 JMX data please follow the steps below.

### Installation

Copy the zip file to `<SilkPerformer_home>\Working` and extract it. A dynaTrace 3.0.xml will be extracted to `<SilkPerformer_home>\Working\Include\jmx-config`.  
Open the realtime.ini located in `<SilkPerformer_home>\Working\Include`. Adapt the file as indicated by the excerpt below.

Add the following line to `"Application In-Depth"`:  
C= JMX:dynaTrace 3.0, Application In-Depth\dynaTrace\dynaTrace 3.0 (JMX MBeanServer),dynaTrace 3.0 (JMX MBeanServer)

    
    
    V= 1.2
    S= Application In-Depth\Borland OptimizeIt ServerTrace 3.0\SNMP, SNMP,
     M= SNMP:1.3.6.1.4.1.1648.500.5.4.0,         OptimizeIt ServerTrace 3.0\JVM Heap Size Total,   	eAvgOnlyCounter,  0,  bytes,   	0,  1, 2, 3, 4, 5,
     M= SNMP:1.3.6.1.4.1.1648.500.5.5.0,         OptimizeIt ServerTrace 3.0\JVM Heap Size Used,    	eAvgOnlyCounter,  0,  bytes,   	0,  1, 2, 3, 4, 5,
     M= SNMP:1.3.6.1.4.1.1648.500.8.4.0,         OptimizeIt ServerTrace 3.0\Virtual Memory Total,   eAvgOnlyCounter,  0,  kbytes,   0,  1, 2, 3, 4, 5,
     M= SNMP:1.3.6.1.4.1.1648.500.8.5.0,         OptimizeIt ServerTrace 3.0\Virtual Memory Used,    eAvgOnlyCounter,  0,  kbytes,   0,  1, 2, 3, 4, 5,
     M= SNMP:1.3.6.1.4.1.1648.500.7.4.0,         OptimizeIt ServerTrace 3.0\Physical Memory Total,  eAvgOnlyCounter,  0,  kbytes,   0,  1, 2, 3, 4, 5,
     M= SNMP:1.3.6.1.4.1.1648.500.7.5.0,         OptimizeIt ServerTrace 3.0\Physical Memory Used,   eAvgOnlyCounter,  0,  kbytes,   0,  1, 2, 3, 4, 5,
     M= SNMP:1.3.6.1.4.1.1648.500.6.4.0,         OptimizeIt ServerTrace 3.0\Paging File Total,     	eAvgOnlyCounter,  0,  mbytes,   0,  1, 2, 3, 4, 5,
     M= SNMP:1.3.6.1.4.1.1648.500.6.5.0,         OptimizeIt ServerTrace 3.0\Paging File Used,     	eAvgOnlyCounter,  0,  mbytes,   0,  1, 2, 3, 4, 5,
     C= JMX:dynaTrace Diagnostics 1.6, Application In-Depth\dynaTrace Diagnostics\dynaTrace Diagnostics 1.6 (JMX MBeanServer),dynaTrace Diagnostics 1.6 (JMX MBeanServer)
     C= JMX:dynaTrace Diagnostics 2.0, Application In-Depth\dynaTrace Diagnostics\dynaTrace Diagnostics 2.0 (JMX MBeanServer),dynaTrace Diagnostics 2.0 (JMX MBeanServer)
     C= JMX:dynaTrace 3.0, Application In-Depth\dynaTrace\dynaTrace 3.0 (JMX MBeanServer),dynaTrace 3.0 (JMX MBeanServer)
    S= Application Server\BEA WebLogic preconfigured\SNMP, ESNMP:BEA-WEBLOGIC-MIB,

The installation is finished and the JMX data of dynaTrace 3.0 is now available in SilkPerformer.

### How can I add dynaTrace 3.0 JMX data?

Open `"Silk Performance Explorer"` and choose `"Monitor"` -> `"Add Data Source"`. In the `"Data Source Wizard"` choose `"Select from predefined Data Source"` and click `"Next"`. Open `"Application in-
Depth"` -> `"dynaTrace"` -> `"dynaTrace 3.0"` and click `"Next"`.

![images_community/download/attachments/11567147/DataSourceWizard.png](images_community/download/attachments/11567147/DataSourceWizard.png)

Fill in the correct data in the connection parameters dialog:

![images_community/download/attachments/11567147/ConnectionParams.png](images_community/download/attachments/11567147/ConnectionParams.png)

Choose the JMX values you want to monitor:

![images_community/download/attachments/11567147/JMXData.png](images_community/download/attachments/11567147/JMXData.png)

