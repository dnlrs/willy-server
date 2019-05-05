
## Configuration file format


```xml
<configuration>
 <anchors n="1">
  <anchor>
   <mac>00:00:00:00:00:00</mac>
   <position>
     <x>0.000000</x>
     <y>0.000000</y>
   </position>
  <anchor>
 </anchors>
 <properties n="1">
  <property>
   <name>property_name</name>
   <value>property_value</value>
  </property>
 </propeties>
</configuration>
```

 Many configurations can be created (also by reading different configuration files), only one will be active.

 Configurations can be loaded from file and saved on file. Defaul configuration file is "_willy_properties.xml_".

 Properties are strings, can be anything as long as the property name is uniform across all modules.

### building NOTE

If any error occurs dugin building like `function not found` within the rapidxml library, go to 
project `properties -> C/C++ -> Language` and set **Conformance Mode** to **No**.