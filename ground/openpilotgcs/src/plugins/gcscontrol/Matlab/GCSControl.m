% GCSCONTROL
%  This class allows the user to send 4-axis stick commands to OpenPilot
%  GCS.
%
% Create class by
%    control = GCSControl
%
% Open connection by 
%    control.connect('01.23.45.67', 89)
% where the first value is the IP address of the computer running GCS and
% the second value is the port on which GCS is listening.
%
% Send command by
%    control.command(pitch, yaw, roll, throttle)
% where all variables are between [-1,1]
%
% Close connection by
%    control.close()

classdef GCSControl < handle
    
    properties
        udpObj;
        isConnected=false;
    end
    
    methods
        function obj=GCSControl()
            obj.isConnected = false;
        end
        function obj=connect(obj,rhost,rport)
            obj.udpObj = udp(rhost,rport);
            fopen(obj.udpObj);
            obj.isConnected = true;
        end
        function obj=command(obj,pitch,yaw,roll,throttle)
            if(obj.isConnected)
                fwrite(obj.udpObj,[42,pitch,yaw,roll,throttle,36],'double')
            end
        end
        function obj=close(obj)
            if(obj.isConnected)
                fclose(obj.udpObj);
                obj.isConnected = false;
            end
        end
    end
end