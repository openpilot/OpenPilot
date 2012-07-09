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