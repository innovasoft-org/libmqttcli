# mqtt
## NAME
&emsp;mqtt - publishes or subscribes packets using MQTT protocol
## SYNOPSIS
&emsp;mqtt _--pub [-I user ID] [-N user name] [-P password] [--buffer-size size] [-h host] [-m message] [-p port] [-t topic] [-v]_  
&emsp;mqtt _--sub [-I user ID] [-N user name] [-P password] [--buffer-size size] [-h host] [-p port] [-t topic] [v]_  
## DESCRIPTION
&emsp;Connects to the broker using specified credentials and publishes to specified topic or starts waiting for subscribed topic.

&emsp;_-I, --userid_  
&emsp;&emsp;Uses specified user ID. By default randomly generated user id is used.  
&emsp;_-N, --username_  
&emsp;&emsp;Uses specified user name. By default none user name is iused.  
# Usage
mqtt --sub -h test.mosquitto.org -p 1884 --username rw --password readwrite -t "#" -v

