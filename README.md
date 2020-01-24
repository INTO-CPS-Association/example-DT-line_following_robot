# case-study_line_follower_robot

## Case Study Description

This example, originally developed in the DESTECS project. The model simulates a robot that can follow a line painted on the ground. The line contrasts from the background and the robot uses a number of sensors to detect light and dark areas on the ground. The robot has two wheels, each powered by individual motors to enable the robot to make controlled changes in direction. The number and position of the sensors may be configured in the model. A controller takes input from the sensors and encoders from the wheels to make outputs to the motors. 

![Line Follower Robot](resources/robot.jpg?raw=true "Line Follower Robot")

The robot moves through a number of phases as it follows a line. At the start of each line is a specific pattern that will be known in advance. Once a genuine line is detected on the ground, the robot follows it until it detects that the end of the line has been reached, when it should go to an idle state. 


## Example Artefacts Overview

The example is available at https://github.com/into-cps/case-study_line_follower_robot in the *master* branch. There are several subfolders for the various elements: DSEs - contains various work in progress DSE scripts to alter CT and DE parameters ; FMU - contains the various FMUs of the study; Models - contains the constituent models defined using the INTO-CPS simulation technologies; Multi-models - contains the multi-model definitions and co-simulation configurations - with 3D and non-3D options, and also with and without the use of replicated FMUs; SysML - contains the SysML models defined for the study; resources - various images for the purposes of this readme file; and userMetricScripts - contains files for DSE analysis. 

The case-study_line_follower_robot folder can be opened in the INTO-CPS application to run the various co-simulations as detailed in this document.

## INTO-CPS Technology

### INTO SysML profile

The multi-model architecture, defined in the INTO-CPS SysML profile, splits the original model into three subsystems, as shown in  the Architecture Structure Diagram. This version comprises *Body* and *Sensor* subsystems and a *Controller* cyber component. The *Body* subsystem, with a target platform as 20-sim as a CT model, comprises the Body, Encoder, Wheel and Motor components. The *Sensor* subsystem contains only Sensor components. The *Controller* component remains the same as the original SysML model. 

![Architecture Structure Diagram](resources/r2g2p_asd.png?raw=true "Architecture Structure Diagram")

Connections are made between the different subsystems, and the underlying components realising the source and destination of values. Two connection diagrams are defined for two block instances *robot2sensor* and *3DRobot*. The first, shown below connects the controller, body and sensor instances:

![Connection Diagram 1](resources/r2g2p_cd.png?raw=true "Connection Diagram")

The second block instance, *3DRobot*, includes a 3DVisualisation block. The other connections remain the same as in the  *robot2sensor* instance.

![Connection Diagram 2](resources/r2g2p_cd_vis.png?raw=true "Connection Diagram")


### Models 

The multi-model produced and analysed using INTO-CPS technology stems from the baseline Crescendo co-model. The multi-model comprises 4 models, splitting the Crescendo model into a multi-CT model, with a single DE model. 

The Crescendo elements (a VDM-RT controller and a 20-sim plant) are largely unchanged, modified so that the 20-sim model is partitioned into 3 high-level submodels: a *body* and two models for the robot *sensors*. By modelling in this way, each submodel can be exported as a separate FMU. For the purposes of multi-modelling, we concentrate on the 20-sim Body subsystem which does not contain the sensors. In their place, the body submodel contains ports for the robot position: **robot_x**, **robot_y**, **robot_z** and **robot_theta**. The other ports  are the same as in the baseline Crescendo model (**total_energy_used**, **servo_left_input** and **servo_right_input**, **wheel_left_rotation** and **wheel_right_rotation**) for interfacing with the controller and visualisation models. The Sensor model contains a link to the map and models a single sensor, taking inputs related to the robot position (**robot_x**, **robot_y**, **robot_z** and **robot_theta**) and returning the current sensor reading(**lf_1_sensor_reading**). 

The VDM-RT controller model is unchanged from the original Crescendo controller.

An OpenModelica model has been defined to model the robot sensors - an alternative to the 20-sim sensor model - this model has the same port collection as the 20-sim version.


### Multi-Models

The case study has two multi-models, one without and one with 3D visualisation (*lfr-1* and *lfr-2* respectively). The two multi-models share several connections; with the *lfr-2* version having additional connections to the 3D visualisation FMU. 

The first collection of connections is between the Body 20-sim model and the Controller VDM-RT model. In this collection, there are several connections corresponding to signals for the actuators that power the motors for the wheels:
- from the *Controller* **servo_left** variable of type real to the **servo_left_input** port of the *Body*; and
- from the *Controller* **servo_right** variable of type real to the **servo_right_input** port of the *Body*

The second collection of connections is present between the Sensor models and the Controller VDM-RT model. For each sensor there is one connection to the controller to represent inputs from line-following sensors that can detect areas of light and dark on the ground. Therefore for a two-sensor model there are two connections:
- from the *Sensor1* **lf_1_sensor_reading** port to the *Controller* **lfLeftVal** port; and 
- from the *Sensor2* **lf_1_sensor_reading** port to the *Controller* **lfRightVal** port. 

A third collection of connections exist between the body and the sensors related to the robot position:
- from the *Body* **robot_x** port to the *Sensor1* and *Sensor2* **robot_x** port; 
- from the *Body* **robot_y** port to the *Sensor1* and *Sensor2* **robot_y** port; 
- from the *Body* **robot_z** port to the *Sensor1* and *Sensor2* **robot_z** port; and 
- from the *Body* **robot_theta** port to the *Sensor1* and *Sensor2* **robot_theta** port. 

Four design parameters are present also: the separation of the line-following sensors from the centre line, in metres (**sensor1.lf_position_x** and **sensor2.lf_position_x**); and the distance forward of the line-following sensors from the centre of the robot, in metres (**sensor1.lf_position_y** and **sensor2.lf_position_y**).

The final collection of connections are only present in the *lfr-2* multi-model, enabling 3D visualisation:
- from the *Body* **robot_x** port to the *3D* **animation.robot.position.x** port; 
- from the *Body* **robot_y** port to the *3D* **animation.robot.position.y** port; 
- from the *Body* **robot_theta** port to the *3D* **animation.robot.angle.z** port;
- from the *Sensor1* **lf_1_sensor_reading** port to the *3D* **animation.sensor.lf.left.reading** port; and
- from the *Sensor2* **lf_1_sensor_reading** port to the *3D* **animation.sensor.lf.left.reading** port.

### Co-simulation

For both the *lfr-1* and *lfr-2* multi-models, co-simulations require approximately 25-30 seconds of simulation to traverse the full map, using a step size of 0.01 seconds. 

### Analyses and Experiments

Below we detail some useful experiments to demonstrate features of the INTO-CPS tool chain.

#### Change FMUs/parameters

The case study has several Sensor FMUs. In the multi-model configuration it is possible to swap the FMU allocated to each sensor instance of the multi-model. We can therefore compare the results of co-simulation using 20-sim sensors (*Sensor_Block.fmu*) for OpenModelica sensors (**To appear**), or a combination of the two. 

In addition, there are parameters defined for the two sensors: an x and y position **lf_position_x** and **lf_position_y**. Experiments may be carried out by defining different values for these to model different placement of the sensors on the robot. 

#### Simulations due to previous results 

Simulations can be replayed using different design parameter values to change the position of the robot sensors. Changing these parameters amounts to changing the design of the robot - some value pairs will produce robots which perform in some way better (e.g. have a faster lap time) and others will result in robots which can not follow the line.

#### Design Space Exploration

TBD
