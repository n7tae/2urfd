<?php
/*
Possible values for IPModus

HideIP
ShowFullIP
ShowLast1ByteOfIP
ShowLast2ByteOfIP
ShowLast3ByteOfIP
*/

// Specify timezone for the dashboard:

date_default_timezone_set(trim(file_get_contents("/etc/timezone")));

// You can force the TZ, and reduce overhead, by spelling it out. For example: date_default_timezone_set("America/Phoenix");
// For a list of PHP-approved timezones, see https://www.php.net/manual/en/timezones.php

$Service     = array();
$CallingHome = array();
$PageOptions = array();

$PageOptions['ContactEmail']                         = 'your_email';		    // Support E-Mail address

$PageOptions['DashboardVersion']                     = '2.5.0';       			// Dashboard Version

$PageOptions['PageRefreshActive']                    = true;          			// Activate automatic refresh
$PageOptions['PageRefreshDelay']                     = '10000';       			// Page refresh time in miliseconds


$PageOptions['RepeatersPage'] = array();
$PageOptions['RepeatersPage']['LimitTo']             = 99;            			// Number of Repeaters to show
$PageOptions['RepeatersPage']['IPModus']             = 'ShowLast2ByteOfIP'; 	// See possible options above
$PageOptions['RepeatersPage']['MasqueradeCharacter'] = '*';	        			// Character used for  masquerade


$PageOptions['PeerPage'] = array();
$PageOptions['PeerPage']['LimitTo']                  = 99;            			// Number of peers to show
$PageOptions['PeerPage']['IPModus']                  = 'ShowLast2ByteOfIP';  	// See possible options above
$PageOptions['PeerPage']['MasqueradeCharacter']      = '*';           			// Character used for  masquerade

$PageOptions['LastHeardPage']['LimitTo']             = 39;                      // Number of stations to show

$PageOptions['ModuleNames'] = array();                                			// Module nomination
$PageOptions['ModuleNames']['A']                     = 'Int.';
$PageOptions['ModuleNames']['B']                     = 'Regional';
$PageOptions['ModuleNames']['C']                     = 'National';
$PageOptions['ModuleNames']['D']                     = '';


$PageOptions['MetaDescription']                      = 'URF is a D-Star Reflector System for Ham Radio Operators.';  // Meta Tag Values, usefull for Search Engine
$PageOptions['MetaKeywords']                         = 'Ham Radio, D-Star, XReflector, XLX, XRF, DCS, REF, M17,';    // Meta Tag Values, usefull forSearch Engine
$PageOptions['MetaAuthor']                           = 'LX1IQ';                                                      // Meta Tag Values, usefull for Search Engine
$PageOptions['MetaRevisit']                          = 'After 30 Days';                                              // Meta Tag Values, usefull for Search Engine
$PageOptions['MetaRobots']                           = 'index,follow';                                               // Meta Tag Values, usefull for Search Engine

$PageOptions['UserPage']['ShowFilter']               = true;                                                         // Show Filter on Users page

$Service['PIDFile']                                  = '/var/run/urfd.pid';
$Service['XMLFile']                                  = '/var/log/urfd.xml';

$CallingHome['Active']                               = false;					               // xlx phone home, true or false
$CallingHome['MyDashBoardURL']                       = 'http://your_dashboard';			       // dashboard url
$CallingHome['ServerURL']                            = 'http://xlxapi.rlx.lu/api.php';         // database server, do not change !!!!
$CallingHome['PushDelay']                            = 10;  	                               // push delay in seconds
$CallingHome['Country']                              = "your_country";                         // Country
$CallingHome['Comment']                              = "your_comment"; 				           // Comment. Max 100 character
$CallingHome['HashFile']                             = "/xlxd-ch/callinghome.php";             // Make sure the apache user has read and write permissions in this folder.
$CallingHome['LastCallHomefile']                     = "/xlxd-ch/lastcallhome.php";            // lastcallhome.php can remain in the tmp folder
$CallingHome['OverrideIPAddress']                    = "";                                     // Insert your IP address here. Leave blank for autodetection. No need to enter a fake address.
$CallingHome['InterlinkFile']                        = "/usr/local/etc/urfd.interlink";        // Path to interlink file

?>
