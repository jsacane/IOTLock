In order to use the script, nodejs must be installed. Follow these directions:

npm install mqtt
npm install -g webpack // install webpack  
cd node_modules/mqtt
npm install . // install dev dependencies 
webpack mqtt.js ./browserMqtt.js --output-library mqtt

The above instructions were taken from this website:
https://www.npmjs.com/package/mqtt

This script has only been tested with chrome. Debug the script by pressing F12 on your keyboard and reading any error messages on the console. The console will tell you if the script is running, the connection has been established, or an mqtt topic has been published. 

