# WILSON2 REPRAP
Impressora reprap do tipo wilson2 com algumas modificações de tamanho







## BOM

Reprap Wilson II Parametric build parts list			
	Set these values:		
Unit system:	metric	← Enter “metric” or “sae” here	
Build variation:	II		
Desired print width (X):	400	mm	
Desired print depth (Y):	400	mm	
Desired print height (Z):	450	mm	
			
Item	Quantity	Length / Notes	
2020 Extrusions			
Vertical frame piece (sides)	4	650	mm
Horizontal frame piece (top and bottom)	3	530	mm
Y axis frame piece	2	600	mm
M10 Smooth Rods			
X-Axis	2	570	mm
Y-Axis	2	600	mm
Z-Axis	2	570	mm
Threaded Rods			
8mm lead screw	2	550	mm
Belts & Pulleys			
Y axis GT2 timing belt (w=6mm)	1	1230	mm
X axis GT2 timing belt (w=6mm)	1	1300	mm
		TOTAL	2530
GT2 pulley	2	Want diameter 16-20mm	
Nuts & Bolts			
M5 x 10mm machine screw	55		
M5 t slot nut	55		
M4 x 10mm machine screw	4	These are used for the psu mounting bracket	
M4 x 20mm machine screw	4	For belt idlers and tensioners	
M4 x 25mm machine screw	4	These are used to mount the extruder to the X carriage	
M4 hex nut	6		
M4 lock nut	2	Use for idler bearing assemblies	
M3 x 10mm machine screw	12	Z and Y axis motor mounts	
M3 x 14mm machine screw	19	3 for X motor, 4 for extruder fan, 2 for lcd mount, 4 for lead screw nut mounts, 2 for ramps cooling fan, for for y table	
M3 x 25mm machine screw	6	3 for extruder stepper motor, 1 for fan duct to the extruder, 2 for tensioners	
M3 x 30mm machine screw	6	4 for print bed, 2 for hot end clamp / servo mount	
M3 x 60mm machine screw	2	These are used for the extruder 	
M3 hex nut	22		
M3 lock nut	5	use locknuts for print bed mount and fan duct mount	
M3 washer	10	Need 4 for extruder springs, but have some handy if needed elsewhere	
M2 x 16mm machine screw	9	These and M2 nuts for mounting end stops, 1 for servo mount, and 2 for LM2596 mount	
M2 hex nut	9		
Printed Parts – Required			
X end idler	1	X-end-idler.stl	
X end motor	1	X-end-motor.stl	
X carriage	1	X-carriage.stl	
X belt tensioner	1	X-tensioner.stl	
Y end idler	1	Y-idler.stl	
Y belt tensioner	1	Y-tensioner.stl	
Y end motor	1	Y-motor.stl	
Y table mount	1	Y-table-10.stl	
Z motor holder, left	1	Z-motor-left.stl	
Z motor holder, right	1	Z-motor-right.stl	
Z top frame, left	1	Z-top-left.stl	
Z top frame, right	1	Z-top-right.stl	
Z lower corner brace, front/right, back/left	2	Z-lower-brace-1.stl	
Z lower corner brace, front/left, back/right	2	Z-lower-brace-2.stl	
Y frame brace	4	Y-brace.stl	
Idler bearing guide (two parts makes one guide)	4	Belt-guide-624.stl	
Extruder body / idler / hot end clamp set	1	Extruder-direct-drive.stl	
Servo mount	1	servo_mount.stl	
Servo endstop mount	1	Servo-endstop-holder.stl	
Extruder fan duct	1	Fanduct-e3dlite6.stl	
Printed Parts – Recommended			
Electronics frame mount (for RAMPS)	1	Ramps-mount.stl	
Filament spool support (two pieces)	1	Filament-mount.stl	
SMART display frame mount (one piece)	1	Smartlcd-mount.stl	
12V PSU frame mounting bracket	2	Psu-mount.stl	
Power switch mount	1	Power-switch-mount.stl	
Everything Else			
NEMA17 Stepper motor	5	4-wire bipolar, high torque preferrably	
MDF board (heated bed mount)	1	see drawing for board in doc/	
Heat bed PCB	1	200x300mm MK1	
Heat bed thermistor	1		
Glass bed	1	3mm borosilicate or picture frame glass	
Controller (Mega/RAMPS, Rambo, etc)	1	Arduino Mega 2560 + RAMPS + A4988 drivers * 4	
SMART lcd controller / SD reader	1		
Endstop (limit) switch	3	6mm x 10mm x 20mm, “snap action” levered switch	
Micro servo (9g type)	1	add wire to make ~ 70cm	
LM10UU linear bearing	12		
608ZZ roller bearing	2	these go in the z tops to hold the upper end of the lead screws (ok if you don't have them)	
624RS roller bearing	3	one for each belt idler, one for extruder	
MK7 Drive gear (or similar)	1		
Brass or delrin lead screw nut	2	Probably comes with your lead screws?	
5mm to 8mm flexible shaft couplers	2	aluminum	
40mm x 40mm x 10mm fan	1	12V	
80mm x 80mm x 10mm fan	1	For use with RAMPS frame mount	
Miniature zip ties	20	This number is approximate	
Extruder idler springs	2	High tension, 1” unsprung length, >3mm ID	
Bed leveling springs	4	Lower tension is OK, 1” unsprung, ID > 3mm	
Power Supply	1	Size appropriately for heat bed and extruder	
Hot end	1	E3d lite6 or E3d v6	
Power switch 	1	KCD1-104N (12mm x 19mm body)	
Tools you'll probably need			
Phillips screwdriver			
Small flat head screwdriver			
Metric ruler			
13mm wrench			
Wire cutters			
Small metal (diamond) files	For cleaning up printed parts		
Drill 			
3mm, 5mm, 8mm drill bits	Or 9/64”, 13/64”, and 5/16”		
Allen wrenches	If using socket-cap screws for frame, and for pulleys		
Pliers			
Soldering iron			
Kapton tape (polyimide)			
			
Notes about wires			
The lengths shown are conservative estimates to help you plan for what you'll need			
Extruder heater wire pair	70cm		
Extruder thermistor wire pair	70cm		
Extruder fan wire pair	70cm		
X end stop wire pair	25cm		
Y end stop wire pair	50cm		
Z end stop wire pair	25cm		
X motor wires (4)	25cm		
Y motor wires (4)	50cm		
Z left motor wires (4)	25cm		
Z right motor wires (4)	60cm		
Extruder motor wires (4)	70cm		
Heat bed power wires	60cm	12 Gauge or heavier	
Heat bed thermistor wires	50cm		
![image](https://user-images.githubusercontent.com/3370347/226289954-7ce0c985-0f5a-4745-b050-7044f2fd22f4.png)
