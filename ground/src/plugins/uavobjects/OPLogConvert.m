function [] = OPLogConvert()
    %% Define indices and arrays of structures to hold data

    actuatorcommandIdx = 1;
    ActuatorCommand.timestamp = 0;
    ActuatorCommand(1).Channel = zeros(1,8);
    ActuatorCommand(1).UpdateTime = 0;
    ActuatorCommand(1).MaxUpdateTime = 0;
    ActuatorCommand(1).NumFailedUpdates = 0;

    actuatordesiredIdx = 1;
    ActuatorDesired.timestamp = 0;
    ActuatorDesired(1).Roll = 0;
    ActuatorDesired(1).Pitch = 0;
    ActuatorDesired(1).Yaw = 0;
    ActuatorDesired(1).Throttle = 0;
    ActuatorDesired(1).UpdateTime = 0;
    ActuatorDesired(1).NumLongUpdates = 0;

    actuatorsettingsIdx = 1;
    ActuatorSettings.timestamp = 0;
    ActuatorSettings(1).FixedWingRoll1 = 0;
    ActuatorSettings(1).FixedWingRoll2 = 0;
    ActuatorSettings(1).FixedWingPitch1 = 0;
    ActuatorSettings(1).FixedWingPitch2 = 0;
    ActuatorSettings(1).FixedWingYaw = 0;
    ActuatorSettings(1).FixedWingThrottle = 0;
    ActuatorSettings(1).VTOLMotorN = 0;
    ActuatorSettings(1).VTOLMotorNE = 0;
    ActuatorSettings(1).VTOLMotorE = 0;
    ActuatorSettings(1).VTOLMotorSE = 0;
    ActuatorSettings(1).VTOLMotorS = 0;
    ActuatorSettings(1).VTOLMotorSW = 0;
    ActuatorSettings(1).VTOLMotorW = 0;
    ActuatorSettings(1).VTOLMotorNW = 0;
    ActuatorSettings(1).ChannelUpdateFreq = zeros(1,2);
    ActuatorSettings(1).ChannelMax = zeros(1,8);
    ActuatorSettings(1).ChannelNeutral = zeros(1,8);
    ActuatorSettings(1).ChannelMin = zeros(1,8);
    ActuatorSettings(1).ChannelType = zeros(1,8);
    ActuatorSettings(1).ChannelAddr = zeros(1,8);

    ahrscalibrationIdx = 1;
    AHRSCalibration.timestamp = 0;
    AHRSCalibration(1).measure_var = 0;
    AHRSCalibration(1).accel_bias = zeros(1,3);
    AHRSCalibration(1).accel_scale = zeros(1,3);
    AHRSCalibration(1).accel_var = zeros(1,3);
    AHRSCalibration(1).gyro_bias = zeros(1,3);
    AHRSCalibration(1).gyro_scale = zeros(1,3);
    AHRSCalibration(1).gyro_var = zeros(1,3);
    AHRSCalibration(1).gyro_tempcompfactor = zeros(1,3);
    AHRSCalibration(1).mag_bias = zeros(1,3);
    AHRSCalibration(1).mag_scale = zeros(1,3);
    AHRSCalibration(1).mag_var = zeros(1,3);
    AHRSCalibration(1).vel_var = 0;
    AHRSCalibration(1).pos_var = 0;

    ahrssettingsIdx = 1;
    AHRSSettings.timestamp = 0;
    AHRSSettings(1).Algorithm = 0;
    AHRSSettings(1).Downsampling = 0;
    AHRSSettings(1).UpdatePeriod = 0;
    AHRSSettings(1).BiasCorrectedRaw = 0;
    AHRSSettings(1).YawBias = 0;
    AHRSSettings(1).PitchBias = 0;
    AHRSSettings(1).RollBias = 0;

    ahrsstatusIdx = 1;
    AhrsStatus.timestamp = 0;
    AhrsStatus(1).SerialNumber = zeros(1,8);
    AhrsStatus(1).CPULoad = 0;
    AhrsStatus(1).RunningTime = 0;
    AhrsStatus(1).IdleTimePerCyle = 0;
    AhrsStatus(1).RunningTimePerCyle = 0;
    AhrsStatus(1).DroppedUpdates = 0;
    AhrsStatus(1).LinkRunning = 0;
    AhrsStatus(1).AhrsKickstarts = 0;
    AhrsStatus(1).AhrsCrcErrors = 0;
    AhrsStatus(1).AhrsRetries = 0;
    AhrsStatus(1).AhrsInvalidPackets = 0;
    AhrsStatus(1).OpCrcErrors = 0;
    AhrsStatus(1).OpRetries = 0;
    AhrsStatus(1).OpInvalidPackets = 0;

    attitudeactualIdx = 1;
    AttitudeActual.timestamp = 0;
    AttitudeActual(1).q1 = 0;
    AttitudeActual(1).q2 = 0;
    AttitudeActual(1).q3 = 0;
    AttitudeActual(1).q4 = 0;
    AttitudeActual(1).Roll = 0;
    AttitudeActual(1).Pitch = 0;
    AttitudeActual(1).Yaw = 0;

    attitudedesiredIdx = 1;
    AttitudeDesired.timestamp = 0;
    AttitudeDesired(1).Roll = 0;
    AttitudeDesired(1).Pitch = 0;
    AttitudeDesired(1).Yaw = 0;
    AttitudeDesired(1).Throttle = 0;

    attituderawIdx = 1;
    AttitudeRaw.timestamp = 0;
    AttitudeRaw(1).magnetometers = zeros(1,3);
    AttitudeRaw(1).gyros = zeros(1,3);
    AttitudeRaw(1).gyros_filtered = zeros(1,3);
    AttitudeRaw(1).gyrotemp = zeros(1,2);
    AttitudeRaw(1).accels = zeros(1,3);
    AttitudeRaw(1).accels_filtered = zeros(1,3);

    baroaltitudeIdx = 1;
    BaroAltitude.timestamp = 0;
    BaroAltitude(1).Altitude = 0;
    BaroAltitude(1).Temperature = 0;
    BaroAltitude(1).Pressure = 0;

    batterysettingsIdx = 1;
    BatterySettings.timestamp = 0;
    BatterySettings(1).BatteryVoltage = 0;
    BatterySettings(1).BatteryCapacity = 0;
    BatterySettings(1).BatteryType = 0;
    BatterySettings(1).Calibrations = zeros(1,2);

    firmwareiapobjIdx = 1;
    FirmwareIAPObj.timestamp = 0;
    FirmwareIAPObj(1).Command = 0;
    FirmwareIAPObj(1).Description = zeros(1,100);
    FirmwareIAPObj(1).HWVersion = 0;
    FirmwareIAPObj(1).Target = 0;
    FirmwareIAPObj(1).ArmReset = 0;
    FirmwareIAPObj(1).crc = 0;

    flightbatterystateIdx = 1;
    FlightBatteryState.timestamp = 0;
    FlightBatteryState(1).Voltage = 0;
    FlightBatteryState(1).Current = 0;
    FlightBatteryState(1).PeakCurrent = 0;
    FlightBatteryState(1).AvgCurrent = 0;
    FlightBatteryState(1).ConsumedEnergy = 0;
    FlightBatteryState(1).EstimatedFlightTime = 0;

    flightplancontrolIdx = 1;
    FlightPlanControl.timestamp = 0;
    FlightPlanControl(1).Test = 0;

    flightplansettingsIdx = 1;
    FlightPlanSettings.timestamp = 0;
    FlightPlanSettings(1).Test = 0;

    flightplanstatusIdx = 1;
    FlightPlanStatus.timestamp = 0;
    FlightPlanStatus(1).Status = 0;
    FlightPlanStatus(1).ErrorType = 0;
    FlightPlanStatus(1).ErrorFileID = 0;
    FlightPlanStatus(1).ErrorLineNum = 0;
    FlightPlanStatus(1).Debug = 0;

    flighttelemetrystatsIdx = 1;
    FlightTelemetryStats.timestamp = 0;
    FlightTelemetryStats(1).Status = 0;
    FlightTelemetryStats(1).TxDataRate = 0;
    FlightTelemetryStats(1).RxDataRate = 0;
    FlightTelemetryStats(1).TxFailures = 0;
    FlightTelemetryStats(1).RxFailures = 0;
    FlightTelemetryStats(1).TxRetries = 0;

    gcstelemetrystatsIdx = 1;
    GCSTelemetryStats.timestamp = 0;
    GCSTelemetryStats(1).Status = 0;
    GCSTelemetryStats(1).TxDataRate = 0;
    GCSTelemetryStats(1).RxDataRate = 0;
    GCSTelemetryStats(1).TxFailures = 0;
    GCSTelemetryStats(1).RxFailures = 0;
    GCSTelemetryStats(1).TxRetries = 0;

    gpspositionIdx = 1;
    GPSPosition.timestamp = 0;
    GPSPosition(1).Status = 0;
    GPSPosition(1).Latitude = 0;
    GPSPosition(1).Longitude = 0;
    GPSPosition(1).Altitude = 0;
    GPSPosition(1).GeoidSeparation = 0;
    GPSPosition(1).Heading = 0;
    GPSPosition(1).Groundspeed = 0;
    GPSPosition(1).Satellites = 0;
    GPSPosition(1).PDOP = 0;
    GPSPosition(1).HDOP = 0;
    GPSPosition(1).VDOP = 0;

    gpssatellitesIdx = 1;
    GPSSatellites.timestamp = 0;
    GPSSatellites(1).SatsInView = 0;
    GPSSatellites(1).PRN = zeros(1,16);
    GPSSatellites(1).Elevation = zeros(1,16);
    GPSSatellites(1).Azimuth = zeros(1,16);
    GPSSatellites(1).SNR = zeros(1,16);

    gpstimeIdx = 1;
    GPSTime.timestamp = 0;
    GPSTime(1).Month = 0;
    GPSTime(1).Day = 0;
    GPSTime(1).Year = 0;
    GPSTime(1).Hour = 0;
    GPSTime(1).Minute = 0;
    GPSTime(1).Second = 0;

    guidancesettingsIdx = 1;
    GuidanceSettings.timestamp = 0;
    GuidanceSettings(1).GuidanceMode = 0;
    GuidanceSettings(1).MaxGroundspeed = 0;
    GuidanceSettings(1).GroundVelocityP = 0;
    GuidanceSettings(1).MaxVerticalSpeed = 0;
    GuidanceSettings(1).VertVelocityP = 0;
    GuidanceSettings(1).VelP = 0;
    GuidanceSettings(1).VelI = 0;
    GuidanceSettings(1).VelD = 0;
    GuidanceSettings(1).DownP = 0;
    GuidanceSettings(1).DownI = 0;
    GuidanceSettings(1).DownD = 0;
    GuidanceSettings(1).MaxVelIntegral = 0;
    GuidanceSettings(1).MaxThrottleIntegral = 0;
    GuidanceSettings(1).VelUpdatePeriod = 0;
    GuidanceSettings(1).VelPIDUpdatePeriod = 0;

    homelocationIdx = 1;
    HomeLocation.timestamp = 0;
    HomeLocation(1).Set = 0;
    HomeLocation(1).Latitude = 0;
    HomeLocation(1).Longitude = 0;
    HomeLocation(1).Altitude = 0;
    HomeLocation(1).ECEF = zeros(1,3);
    HomeLocation(1).RNE = zeros(1,9);
    HomeLocation(1).Be = zeros(1,3);

    i2cstatsIdx = 1;
    I2CStats.timestamp = 0;
    I2CStats(1).event_errors = 0;
    I2CStats(1).fsm_errors = 0;
    I2CStats(1).irq_errors = 0;
    I2CStats(1).last_error_type = 0;
    I2CStats(1).evirq_log = zeros(1,5);
    I2CStats(1).erirq_log = zeros(1,5);
    I2CStats(1).event_log = zeros(1,5);
    I2CStats(1).state_log = zeros(1,5);

    manualcontrolcommandIdx = 1;
    ManualControlCommand.timestamp = 0;
    ManualControlCommand(1).Connected = 0;
    ManualControlCommand(1).Armed = 0;
    ManualControlCommand(1).Roll = 0;
    ManualControlCommand(1).Pitch = 0;
    ManualControlCommand(1).Yaw = 0;
    ManualControlCommand(1).Throttle = 0;
    ManualControlCommand(1).FlightMode = 0;
    ManualControlCommand(1).StabilizationSettings = zeros(1,3);
    ManualControlCommand(1).Accessory1 = 0;
    ManualControlCommand(1).Accessory2 = 0;
    ManualControlCommand(1).Accessory3 = 0;
    ManualControlCommand(1).Channel = zeros(1,8);

    manualcontrolsettingsIdx = 1;
    ManualControlSettings.timestamp = 0;
    ManualControlSettings(1).InputMode = 0;
    ManualControlSettings(1).Roll = 0;
    ManualControlSettings(1).Pitch = 0;
    ManualControlSettings(1).Yaw = 0;
    ManualControlSettings(1).Throttle = 0;
    ManualControlSettings(1).FlightMode = 0;
    ManualControlSettings(1).Accessory1 = 0;
    ManualControlSettings(1).Accessory2 = 0;
    ManualControlSettings(1).Accessory3 = 0;
    ManualControlSettings(1).Pos1StabilizationSettings = zeros(1,3);
    ManualControlSettings(1).Pos2StabilizationSettings = zeros(1,3);
    ManualControlSettings(1).Pos3StabilizationSettings = zeros(1,3);
    ManualControlSettings(1).Pos1FlightMode = 0;
    ManualControlSettings(1).Pos2FlightMode = 0;
    ManualControlSettings(1).Pos3FlightMode = 0;
    ManualControlSettings(1).ChannelMax = zeros(1,8);
    ManualControlSettings(1).ChannelNeutral = zeros(1,8);
    ManualControlSettings(1).ChannelMin = zeros(1,8);
    ManualControlSettings(1).ArmedTimeout = 0;

    mixersettingsIdx = 1;
    MixerSettings.timestamp = 0;
    MixerSettings(1).MaxAccel = 0;
    MixerSettings(1).FeedForward = 0;
    MixerSettings(1).AccelTime = 0;
    MixerSettings(1).DecelTime = 0;
    MixerSettings(1).ThrottleCurve1 = zeros(1,5);
    MixerSettings(1).ThrottleCurve2 = zeros(1,5);
    MixerSettings(1).Mixer1Type = 0;
    MixerSettings(1).Mixer1Vector = zeros(1,5);
    MixerSettings(1).Mixer2Type = 0;
    MixerSettings(1).Mixer2Vector = zeros(1,5);
    MixerSettings(1).Mixer3Type = 0;
    MixerSettings(1).Mixer3Vector = zeros(1,5);
    MixerSettings(1).Mixer4Type = 0;
    MixerSettings(1).Mixer4Vector = zeros(1,5);
    MixerSettings(1).Mixer5Type = 0;
    MixerSettings(1).Mixer5Vector = zeros(1,5);
    MixerSettings(1).Mixer6Type = 0;
    MixerSettings(1).Mixer6Vector = zeros(1,5);
    MixerSettings(1).Mixer7Type = 0;
    MixerSettings(1).Mixer7Vector = zeros(1,5);
    MixerSettings(1).Mixer8Type = 0;
    MixerSettings(1).Mixer8Vector = zeros(1,5);

    mixerstatusIdx = 1;
    MixerStatus.timestamp = 0;
    MixerStatus(1).Mixer1 = 0;
    MixerStatus(1).Mixer2 = 0;
    MixerStatus(1).Mixer3 = 0;
    MixerStatus(1).Mixer4 = 0;
    MixerStatus(1).Mixer5 = 0;
    MixerStatus(1).Mixer6 = 0;
    MixerStatus(1).Mixer7 = 0;
    MixerStatus(1).Mixer8 = 0;

    objectpersistenceIdx = 1;
    ObjectPersistence.timestamp = 0;
    ObjectPersistence(1).Operation = 0;
    ObjectPersistence(1).Selection = 0;
    ObjectPersistence(1).ObjectID = 0;
    ObjectPersistence(1).InstanceID = 0;

    pipxtrememodemsettingsIdx = 1;
    PipXtremeModemSettings.timestamp = 0;
    PipXtremeModemSettings(1).Mode = 0;
    PipXtremeModemSettings(1).Serial_Baudrate = 0;
    PipXtremeModemSettings(1).Frequency_Calibration = 0;
    PipXtremeModemSettings(1).Frequency_Min = 0;
    PipXtremeModemSettings(1).Frequency_Max = 0;
    PipXtremeModemSettings(1).Frequency = 0;
    PipXtremeModemSettings(1).Max_RF_Bandwidth = 0;
    PipXtremeModemSettings(1).Max_Tx_Power = 0;
    PipXtremeModemSettings(1).Tx_Data_Wait = 0;
    PipXtremeModemSettings(1).AES_Encryption = 0;
    PipXtremeModemSettings(1).AES_EncryptionKey = zeros(1,16);
    PipXtremeModemSettings(1).Paired_Serial_Number = 0;

    pipxtrememodemstatusIdx = 1;
    PipXtremeModemStatus.timestamp = 0;
    PipXtremeModemStatus(1).Firmware_Version_Major = 0;
    PipXtremeModemStatus(1).Firmware_Version_Minor = 0;
    PipXtremeModemStatus(1).Serial_Number = 0;
    PipXtremeModemStatus(1).Up_Time = 0;
    PipXtremeModemStatus(1).Frequency = 0;
    PipXtremeModemStatus(1).RF_Bandwidth = 0;
    PipXtremeModemStatus(1).Tx_Power = 0;
    PipXtremeModemStatus(1).State = 0;
    PipXtremeModemStatus(1).Tx_Retry = 0;
    PipXtremeModemStatus(1).Tx_Data_Rate = 0;
    PipXtremeModemStatus(1).Rx_Data_Rate = 0;

    positionactualIdx = 1;
    PositionActual.timestamp = 0;
    PositionActual(1).North = 0;
    PositionActual(1).East = 0;
    PositionActual(1).Down = 0;

    positiondesiredIdx = 1;
    PositionDesired.timestamp = 0;
    PositionDesired(1).North = 0;
    PositionDesired(1).East = 0;
    PositionDesired(1).Down = 0;

    ratedesiredIdx = 1;
    RateDesired.timestamp = 0;
    RateDesired(1).Roll = 0;
    RateDesired(1).Pitch = 0;
    RateDesired(1).Yaw = 0;

    stabilizationsettingsIdx = 1;
    StabilizationSettings.timestamp = 0;
    StabilizationSettings(1).RollMax = 0;
    StabilizationSettings(1).PitchMax = 0;
    StabilizationSettings(1).YawMax = 0;
    StabilizationSettings(1).ManualRate = zeros(1,3);
    StabilizationSettings(1).MaximumRate = zeros(1,3);
    StabilizationSettings(1).RollRatePI = zeros(1,3);
    StabilizationSettings(1).PitchRatePI = zeros(1,3);
    StabilizationSettings(1).YawRatePI = zeros(1,3);
    StabilizationSettings(1).RollPI = zeros(1,3);
    StabilizationSettings(1).PitchPI = zeros(1,3);
    StabilizationSettings(1).YawPI = zeros(1,3);

    systemalarmsIdx = 1;
    SystemAlarms.timestamp = 0;
    SystemAlarms(1).Alarm = zeros(1,14);

    systemsettingsIdx = 1;
    SystemSettings.timestamp = 0;
    SystemSettings(1).AirframeType = 0;

    systemstatsIdx = 1;
    SystemStats.timestamp = 0;
    SystemStats(1).FlightTime = 0;
    SystemStats(1).HeapRemaining = 0;
    SystemStats(1).CPULoad = 0;

    taskinfoIdx = 1;
    TaskInfo.timestamp = 0;
    TaskInfo(1).StackRemaining = zeros(1,13);
    TaskInfo(1).Running = zeros(1,13);

    telemetrysettingsIdx = 1;
    TelemetrySettings.timestamp = 0;
    TelemetrySettings(1).Speed = 0;

    velocityactualIdx = 1;
    VelocityActual.timestamp = 0;
    VelocityActual(1).North = 0;
    VelocityActual(1).East = 0;
    VelocityActual(1).Down = 0;

    velocitydesiredIdx = 1;
    VelocityDesired.timestamp = 0;
    VelocityDesired(1).North = 0;
    VelocityDesired(1).East = 0;
    VelocityDesired(1).Down = 0;

    watchdogstatusIdx = 1;
    WatchdogStatus.timestamp = 0;
    WatchdogStatus(1).BootupFlags = 0;
    WatchdogStatus(1).ActiveFlags = 0;

   
    %% Open file
    %fid = fopen('log.opl');
    [FileName,PathName,FilterIndex] = uigetfile('*.opl');
    logfile = strcat(PathName,FileName);
    fid = fopen(logfile);
    
    while (1)
        %% Read logging header        
        timestamp = fread(fid, 1, 'uint32');
        if (feof(fid)); break; end
        datasize = fread(fid, 1, 'int64');
          
        
        %% Read message header        
        % get sync field (0x3C, 1 byte)
        sync = fread(fid, 1, 'uint8');
        if sync ~= hex2dec('3C')
            disp ('Wrong sync byte');
            return
        end        
        % get msg type (quint8 1 byte ) should be 0x20, ignore the rest?
        msgType = fread(fid, 1, 'uint8');
        if msgType ~= hex2dec('20')
            disp ('Wrong msgType');
            return
        end        
        % get msg size (quint16 2 bytes) excludes crc, include msg header and data payload
        msgSize = fread(fid, 1, 'uint16');        
        % get obj id (quint32 4 bytes)
        objID = fread(fid, 1, 'uint32');        
        
        
        %% Read object
        switch objID
            case 3907024856
                ActuatorCommand(actuatorcommandIdx) = ReadActuatorCommandObject(fid, timestamp);
                actuatorcommandIdx = actuatorcommandIdx + 1;
            case 3562104706
                ActuatorDesired(actuatordesiredIdx) = ReadActuatorDesiredObject(fid, timestamp);
                actuatordesiredIdx = actuatordesiredIdx + 1;
            case 844831578
                ActuatorSettings(actuatorsettingsIdx) = ReadActuatorSettingsObject(fid, timestamp);
                actuatorsettingsIdx = actuatorsettingsIdx + 1;
            case 806362034
                AHRSCalibration(ahrscalibrationIdx) = ReadAHRSCalibrationObject(fid, timestamp);
                ahrscalibrationIdx = ahrscalibrationIdx + 1;
            case 3741078856
                AHRSSettings(ahrssettingsIdx) = ReadAHRSSettingsObject(fid, timestamp);
                ahrssettingsIdx = ahrssettingsIdx + 1;
            case 933623714
                AhrsStatus(ahrsstatusIdx) = ReadAhrsStatusObject(fid, timestamp);
                ahrsstatusIdx = ahrsstatusIdx + 1;
            case 4233858292
                AttitudeActual(attitudeactualIdx) = ReadAttitudeActualObject(fid, timestamp);
                attitudeactualIdx = attitudeactualIdx + 1;
            case 1412270808
                AttitudeDesired(attitudedesiredIdx) = ReadAttitudeDesiredObject(fid, timestamp);
                attitudedesiredIdx = attitudedesiredIdx + 1;
            case 1323193976
                AttitudeRaw(attituderawIdx) = ReadAttitudeRawObject(fid, timestamp);
                attituderawIdx = attituderawIdx + 1;
            case 3980666102
                BaroAltitude(baroaltitudeIdx) = ReadBaroAltitudeObject(fid, timestamp);
                baroaltitudeIdx = baroaltitudeIdx + 1;
            case 2784959898
                BatterySettings(batterysettingsIdx) = ReadBatterySettingsObject(fid, timestamp);
                batterysettingsIdx = batterysettingsIdx + 1;
            case 879185696
                FirmwareIAPObj(firmwareiapobjIdx) = ReadFirmwareIAPObjObject(fid, timestamp);
                firmwareiapobjIdx = firmwareiapobjIdx + 1;
            case 126985486
                FlightBatteryState(flightbatterystateIdx) = ReadFlightBatteryStateObject(fid, timestamp);
                flightbatterystateIdx = flightbatterystateIdx + 1;
            case 4125349796
                FlightPlanControl(flightplancontrolIdx) = ReadFlightPlanControlObject(fid, timestamp);
                flightplancontrolIdx = flightplancontrolIdx + 1;
            case 2234942498
                FlightPlanSettings(flightplansettingsIdx) = ReadFlightPlanSettingsObject(fid, timestamp);
                flightplansettingsIdx = flightplansettingsIdx + 1;
            case 2726772894
                FlightPlanStatus(flightplanstatusIdx) = ReadFlightPlanStatusObject(fid, timestamp);
                flightplanstatusIdx = flightplanstatusIdx + 1;
            case 1712072286
                FlightTelemetryStats(flighttelemetrystatsIdx) = ReadFlightTelemetryStatsObject(fid, timestamp);
                flighttelemetrystatsIdx = flighttelemetrystatsIdx + 1;
            case 1998458950
                GCSTelemetryStats(gcstelemetrystatsIdx) = ReadGCSTelemetryStatsObject(fid, timestamp);
                gcstelemetrystatsIdx = gcstelemetrystatsIdx + 1;
            case 3041480770
                GPSPosition(gpspositionIdx) = ReadGPSPositionObject(fid, timestamp);
                gpspositionIdx = gpspositionIdx + 1;
            case 3593446318
                GPSSatellites(gpssatellitesIdx) = ReadGPSSatellitesObject(fid, timestamp);
                gpssatellitesIdx = gpssatellitesIdx + 1;
            case 1459613858
                GPSTime(gpstimeIdx) = ReadGPSTimeObject(fid, timestamp);
                gpstimeIdx = gpstimeIdx + 1;
            case 2071403670
                GuidanceSettings(guidancesettingsIdx) = ReadGuidanceSettingsObject(fid, timestamp);
                guidancesettingsIdx = guidancesettingsIdx + 1;
            case 3590360786
                HomeLocation(homelocationIdx) = ReadHomeLocationObject(fid, timestamp);
                homelocationIdx = homelocationIdx + 1;
            case 122889918
                I2CStats(i2cstatsIdx) = ReadI2CStatsObject(fid, timestamp);
                i2cstatsIdx = i2cstatsIdx + 1;
            case 2841592332
                ManualControlCommand(manualcontrolcommandIdx) = ReadManualControlCommandObject(fid, timestamp);
                manualcontrolcommandIdx = manualcontrolcommandIdx + 1;
            case 157988682
                ManualControlSettings(manualcontrolsettingsIdx) = ReadManualControlSettingsObject(fid, timestamp);
                manualcontrolsettingsIdx = manualcontrolsettingsIdx + 1;
            case 1336817486
                MixerSettings(mixersettingsIdx) = ReadMixerSettingsObject(fid, timestamp);
                mixersettingsIdx = mixersettingsIdx + 1;
            case 4137893648
                MixerStatus(mixerstatusIdx) = ReadMixerStatusObject(fid, timestamp);
                mixerstatusIdx = mixerstatusIdx + 1;
            case 572614706
                ObjectPersistence(objectpersistenceIdx) = ReadObjectPersistenceObject(fid, timestamp);
                objectpersistenceIdx = objectpersistenceIdx + 1;
            case 444830632
                PipXtremeModemSettings(pipxtrememodemsettingsIdx) = ReadPipXtremeModemSettingsObject(fid, timestamp);
                pipxtrememodemsettingsIdx = pipxtrememodemsettingsIdx + 1;
            case 2490854928
                PipXtremeModemStatus(pipxtrememodemstatusIdx) = ReadPipXtremeModemStatusObject(fid, timestamp);
                pipxtrememodemstatusIdx = pipxtrememodemstatusIdx + 1;
            case 3765671478
                PositionActual(positionactualIdx) = ReadPositionActualObject(fid, timestamp);
                positionactualIdx = positionactualIdx + 1;
            case 801433018
                PositionDesired(positiondesiredIdx) = ReadPositionDesiredObject(fid, timestamp);
                positiondesiredIdx = positiondesiredIdx + 1;
            case 3124868380
                RateDesired(ratedesiredIdx) = ReadRateDesiredObject(fid, timestamp);
                ratedesiredIdx = ratedesiredIdx + 1;
            case 3792991236
                StabilizationSettings(stabilizationsettingsIdx) = ReadStabilizationSettingsObject(fid, timestamp);
                stabilizationsettingsIdx = stabilizationsettingsIdx + 1;
            case 2311314672
                SystemAlarms(systemalarmsIdx) = ReadSystemAlarmsObject(fid, timestamp);
                systemalarmsIdx = systemalarmsIdx + 1;
            case 59202798
                SystemSettings(systemsettingsIdx) = ReadSystemSettingsObject(fid, timestamp);
                systemsettingsIdx = systemsettingsIdx + 1;
            case 680908530
                SystemStats(systemstatsIdx) = ReadSystemStatsObject(fid, timestamp);
                systemstatsIdx = systemstatsIdx + 1;
            case 1358273008
                TaskInfo(taskinfoIdx) = ReadTaskInfoObject(fid, timestamp);
                taskinfoIdx = taskinfoIdx + 1;
            case 2785592614
                TelemetrySettings(telemetrysettingsIdx) = ReadTelemetrySettingsObject(fid, timestamp);
                telemetrysettingsIdx = telemetrysettingsIdx + 1;
            case 1207999624
                VelocityActual(velocityactualIdx) = ReadVelocityActualObject(fid, timestamp);
                velocityactualIdx = velocityactualIdx + 1;
            case 305094202
                VelocityDesired(velocitydesiredIdx) = ReadVelocityDesiredObject(fid, timestamp);
                velocitydesiredIdx = velocitydesiredIdx + 1;
            case 3594971408
                WatchdogStatus(watchdogstatusIdx) = ReadWatchdogStatusObject(fid, timestamp);
                watchdogstatusIdx = watchdogstatusIdx + 1;
                
            otherwise
                disp('Unknown object ID');
                msgBytesLeft = datasize - 1 - 1 - 2 - 4;
                fread(fid, msgBytesLeft, 'uint8');
        end
        
    end
    
    %% Clean Up and Save mat file
    fclose(fid);
    
    matfile = strrep(logfile,'opl','mat');
    save(matfile ,'ActuatorCommand','ActuatorDesired','ActuatorSettings','AHRSCalibration','AHRSSettings','AhrsStatus','AttitudeActual','AttitudeDesired','AttitudeRaw','BaroAltitude','BatterySettings','FirmwareIAPObj','FlightBatteryState','FlightPlanControl','FlightPlanSettings','FlightPlanStatus','FlightTelemetryStats','GCSTelemetryStats','GPSPosition','GPSSatellites','GPSTime','GuidanceSettings','HomeLocation','I2CStats','ManualControlCommand','ManualControlSettings','MixerSettings','MixerStatus','ObjectPersistence','PipXtremeModemSettings','PipXtremeModemStatus','PositionActual','PositionDesired','RateDesired','StabilizationSettings','SystemAlarms','SystemSettings','SystemStats','TaskInfo','TelemetrySettings','VelocityActual','VelocityDesired','WatchdogStatus');
    
end


%% Object reading functions
function [ActuatorCommand] = ReadActuatorCommandObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        ActuatorCommand.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    ActuatorCommand.timestamp = timestamp;
    ActuatorCommand.Channel = double(fread(fid, 8, 'int16'));
    ActuatorCommand.UpdateTime = double(fread(fid, 1, 'uint8'));
    ActuatorCommand.MaxUpdateTime = double(fread(fid, 1, 'uint8'));
    ActuatorCommand.NumFailedUpdates = double(fread(fid, 1, 'uint8'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [ActuatorDesired] = ReadActuatorDesiredObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        ActuatorDesired.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    ActuatorDesired.timestamp = timestamp;
    ActuatorDesired.Roll = double(fread(fid, 1, 'float32'));
    ActuatorDesired.Pitch = double(fread(fid, 1, 'float32'));
    ActuatorDesired.Yaw = double(fread(fid, 1, 'float32'));
    ActuatorDesired.Throttle = double(fread(fid, 1, 'float32'));
    ActuatorDesired.UpdateTime = double(fread(fid, 1, 'float32'));
    ActuatorDesired.NumLongUpdates = double(fread(fid, 1, 'float32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [ActuatorSettings] = ReadActuatorSettingsObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        ActuatorSettings.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    ActuatorSettings.timestamp = timestamp;
    ActuatorSettings.FixedWingRoll1 = double(fread(fid, 1, 'uint8'));
    ActuatorSettings.FixedWingRoll2 = double(fread(fid, 1, 'uint8'));
    ActuatorSettings.FixedWingPitch1 = double(fread(fid, 1, 'uint8'));
    ActuatorSettings.FixedWingPitch2 = double(fread(fid, 1, 'uint8'));
    ActuatorSettings.FixedWingYaw = double(fread(fid, 1, 'uint8'));
    ActuatorSettings.FixedWingThrottle = double(fread(fid, 1, 'uint8'));
    ActuatorSettings.VTOLMotorN = double(fread(fid, 1, 'uint8'));
    ActuatorSettings.VTOLMotorNE = double(fread(fid, 1, 'uint8'));
    ActuatorSettings.VTOLMotorE = double(fread(fid, 1, 'uint8'));
    ActuatorSettings.VTOLMotorSE = double(fread(fid, 1, 'uint8'));
    ActuatorSettings.VTOLMotorS = double(fread(fid, 1, 'uint8'));
    ActuatorSettings.VTOLMotorSW = double(fread(fid, 1, 'uint8'));
    ActuatorSettings.VTOLMotorW = double(fread(fid, 1, 'uint8'));
    ActuatorSettings.VTOLMotorNW = double(fread(fid, 1, 'uint8'));
    ActuatorSettings.ChannelUpdateFreq = double(fread(fid, 2, 'int16'));
    ActuatorSettings.ChannelMax = double(fread(fid, 8, 'int16'));
    ActuatorSettings.ChannelNeutral = double(fread(fid, 8, 'int16'));
    ActuatorSettings.ChannelMin = double(fread(fid, 8, 'int16'));
    ActuatorSettings.ChannelType = double(fread(fid, 8, 'uint8'));
    ActuatorSettings.ChannelAddr = double(fread(fid, 8, 'uint8'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [AHRSCalibration] = ReadAHRSCalibrationObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        AHRSCalibration.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    AHRSCalibration.timestamp = timestamp;
    AHRSCalibration.measure_var = double(fread(fid, 1, 'uint8'));
    AHRSCalibration.accel_bias = double(fread(fid, 3, 'float32'));
    AHRSCalibration.accel_scale = double(fread(fid, 3, 'float32'));
    AHRSCalibration.accel_var = double(fread(fid, 3, 'float32'));
    AHRSCalibration.gyro_bias = double(fread(fid, 3, 'float32'));
    AHRSCalibration.gyro_scale = double(fread(fid, 3, 'float32'));
    AHRSCalibration.gyro_var = double(fread(fid, 3, 'float32'));
    AHRSCalibration.gyro_tempcompfactor = double(fread(fid, 3, 'float32'));
    AHRSCalibration.mag_bias = double(fread(fid, 3, 'float32'));
    AHRSCalibration.mag_scale = double(fread(fid, 3, 'float32'));
    AHRSCalibration.mag_var = double(fread(fid, 3, 'float32'));
    AHRSCalibration.vel_var = double(fread(fid, 1, 'float32'));
    AHRSCalibration.pos_var = double(fread(fid, 1, 'float32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [AHRSSettings] = ReadAHRSSettingsObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        AHRSSettings.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    AHRSSettings.timestamp = timestamp;
    AHRSSettings.Algorithm = double(fread(fid, 1, 'uint8'));
    AHRSSettings.Downsampling = double(fread(fid, 1, 'uint8'));
    AHRSSettings.UpdatePeriod = double(fread(fid, 1, 'uint8'));
    AHRSSettings.BiasCorrectedRaw = double(fread(fid, 1, 'uint8'));
    AHRSSettings.YawBias = double(fread(fid, 1, 'float32'));
    AHRSSettings.PitchBias = double(fread(fid, 1, 'float32'));
    AHRSSettings.RollBias = double(fread(fid, 1, 'float32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [AhrsStatus] = ReadAhrsStatusObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        AhrsStatus.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    AhrsStatus.timestamp = timestamp;
    AhrsStatus.SerialNumber = double(fread(fid, 8, 'uint8'));
    AhrsStatus.CPULoad = double(fread(fid, 1, 'uint8'));
    AhrsStatus.RunningTime = double(fread(fid, 1, 'uint32'));
    AhrsStatus.IdleTimePerCyle = double(fread(fid, 1, 'uint8'));
    AhrsStatus.RunningTimePerCyle = double(fread(fid, 1, 'uint8'));
    AhrsStatus.DroppedUpdates = double(fread(fid, 1, 'uint8'));
    AhrsStatus.LinkRunning = double(fread(fid, 1, 'uint8'));
    AhrsStatus.AhrsKickstarts = double(fread(fid, 1, 'uint8'));
    AhrsStatus.AhrsCrcErrors = double(fread(fid, 1, 'uint8'));
    AhrsStatus.AhrsRetries = double(fread(fid, 1, 'uint8'));
    AhrsStatus.AhrsInvalidPackets = double(fread(fid, 1, 'uint8'));
    AhrsStatus.OpCrcErrors = double(fread(fid, 1, 'uint8'));
    AhrsStatus.OpRetries = double(fread(fid, 1, 'uint8'));
    AhrsStatus.OpInvalidPackets = double(fread(fid, 1, 'uint8'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [AttitudeActual] = ReadAttitudeActualObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        AttitudeActual.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    AttitudeActual.timestamp = timestamp;
    AttitudeActual.q1 = double(fread(fid, 1, 'float32'));
    AttitudeActual.q2 = double(fread(fid, 1, 'float32'));
    AttitudeActual.q3 = double(fread(fid, 1, 'float32'));
    AttitudeActual.q4 = double(fread(fid, 1, 'float32'));
    AttitudeActual.Roll = double(fread(fid, 1, 'float32'));
    AttitudeActual.Pitch = double(fread(fid, 1, 'float32'));
    AttitudeActual.Yaw = double(fread(fid, 1, 'float32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [AttitudeDesired] = ReadAttitudeDesiredObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        AttitudeDesired.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    AttitudeDesired.timestamp = timestamp;
    AttitudeDesired.Roll = double(fread(fid, 1, 'float32'));
    AttitudeDesired.Pitch = double(fread(fid, 1, 'float32'));
    AttitudeDesired.Yaw = double(fread(fid, 1, 'float32'));
    AttitudeDesired.Throttle = double(fread(fid, 1, 'float32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [AttitudeRaw] = ReadAttitudeRawObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        AttitudeRaw.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    AttitudeRaw.timestamp = timestamp;
    AttitudeRaw.magnetometers = double(fread(fid, 3, 'int16'));
    AttitudeRaw.gyros = double(fread(fid, 3, 'uint16'));
    AttitudeRaw.gyros_filtered = double(fread(fid, 3, 'float32'));
    AttitudeRaw.gyrotemp = double(fread(fid, 2, 'uint16'));
    AttitudeRaw.accels = double(fread(fid, 3, 'uint16'));
    AttitudeRaw.accels_filtered = double(fread(fid, 3, 'float32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [BaroAltitude] = ReadBaroAltitudeObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        BaroAltitude.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    BaroAltitude.timestamp = timestamp;
    BaroAltitude.Altitude = double(fread(fid, 1, 'float32'));
    BaroAltitude.Temperature = double(fread(fid, 1, 'float32'));
    BaroAltitude.Pressure = double(fread(fid, 1, 'float32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [BatterySettings] = ReadBatterySettingsObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        BatterySettings.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    BatterySettings.timestamp = timestamp;
    BatterySettings.BatteryVoltage = double(fread(fid, 1, 'float32'));
    BatterySettings.BatteryCapacity = double(fread(fid, 1, 'uint32'));
    BatterySettings.BatteryType = double(fread(fid, 1, 'uint8'));
    BatterySettings.Calibrations = double(fread(fid, 2, 'float32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [FirmwareIAPObj] = ReadFirmwareIAPObjObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        FirmwareIAPObj.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    FirmwareIAPObj.timestamp = timestamp;
    FirmwareIAPObj.Command = double(fread(fid, 1, 'uint16'));
    FirmwareIAPObj.Description = double(fread(fid, 100, 'uint8'));
    FirmwareIAPObj.HWVersion = double(fread(fid, 1, 'uint8'));
    FirmwareIAPObj.Target = double(fread(fid, 1, 'uint8'));
    FirmwareIAPObj.ArmReset = double(fread(fid, 1, 'uint8'));
    FirmwareIAPObj.crc = double(fread(fid, 1, 'uint32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [FlightBatteryState] = ReadFlightBatteryStateObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        FlightBatteryState.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    FlightBatteryState.timestamp = timestamp;
    FlightBatteryState.Voltage = double(fread(fid, 1, 'float32'));
    FlightBatteryState.Current = double(fread(fid, 1, 'float32'));
    FlightBatteryState.PeakCurrent = double(fread(fid, 1, 'float32'));
    FlightBatteryState.AvgCurrent = double(fread(fid, 1, 'float32'));
    FlightBatteryState.ConsumedEnergy = double(fread(fid, 1, 'float32'));
    FlightBatteryState.EstimatedFlightTime = double(fread(fid, 1, 'float32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [FlightPlanControl] = ReadFlightPlanControlObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        FlightPlanControl.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    FlightPlanControl.timestamp = timestamp;
    FlightPlanControl.Test = double(fread(fid, 1, 'float32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [FlightPlanSettings] = ReadFlightPlanSettingsObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        FlightPlanSettings.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    FlightPlanSettings.timestamp = timestamp;
    FlightPlanSettings.Test = double(fread(fid, 1, 'float32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [FlightPlanStatus] = ReadFlightPlanStatusObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        FlightPlanStatus.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    FlightPlanStatus.timestamp = timestamp;
    FlightPlanStatus.Status = double(fread(fid, 1, 'uint8'));
    FlightPlanStatus.ErrorType = double(fread(fid, 1, 'uint8'));
    FlightPlanStatus.ErrorFileID = double(fread(fid, 1, 'uint32'));
    FlightPlanStatus.ErrorLineNum = double(fread(fid, 1, 'uint32'));
    FlightPlanStatus.Debug = double(fread(fid, 1, 'float32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [FlightTelemetryStats] = ReadFlightTelemetryStatsObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        FlightTelemetryStats.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    FlightTelemetryStats.timestamp = timestamp;
    FlightTelemetryStats.Status = double(fread(fid, 1, 'uint8'));
    FlightTelemetryStats.TxDataRate = double(fread(fid, 1, 'float32'));
    FlightTelemetryStats.RxDataRate = double(fread(fid, 1, 'float32'));
    FlightTelemetryStats.TxFailures = double(fread(fid, 1, 'uint32'));
    FlightTelemetryStats.RxFailures = double(fread(fid, 1, 'uint32'));
    FlightTelemetryStats.TxRetries = double(fread(fid, 1, 'uint32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [GCSTelemetryStats] = ReadGCSTelemetryStatsObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        GCSTelemetryStats.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    GCSTelemetryStats.timestamp = timestamp;
    GCSTelemetryStats.Status = double(fread(fid, 1, 'uint8'));
    GCSTelemetryStats.TxDataRate = double(fread(fid, 1, 'float32'));
    GCSTelemetryStats.RxDataRate = double(fread(fid, 1, 'float32'));
    GCSTelemetryStats.TxFailures = double(fread(fid, 1, 'uint32'));
    GCSTelemetryStats.RxFailures = double(fread(fid, 1, 'uint32'));
    GCSTelemetryStats.TxRetries = double(fread(fid, 1, 'uint32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [GPSPosition] = ReadGPSPositionObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        GPSPosition.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    GPSPosition.timestamp = timestamp;
    GPSPosition.Status = double(fread(fid, 1, 'uint8'));
    GPSPosition.Latitude = double(fread(fid, 1, 'int32'));
    GPSPosition.Longitude = double(fread(fid, 1, 'int32'));
    GPSPosition.Altitude = double(fread(fid, 1, 'float32'));
    GPSPosition.GeoidSeparation = double(fread(fid, 1, 'float32'));
    GPSPosition.Heading = double(fread(fid, 1, 'float32'));
    GPSPosition.Groundspeed = double(fread(fid, 1, 'float32'));
    GPSPosition.Satellites = double(fread(fid, 1, 'int8'));
    GPSPosition.PDOP = double(fread(fid, 1, 'float32'));
    GPSPosition.HDOP = double(fread(fid, 1, 'float32'));
    GPSPosition.VDOP = double(fread(fid, 1, 'float32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [GPSSatellites] = ReadGPSSatellitesObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        GPSSatellites.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    GPSSatellites.timestamp = timestamp;
    GPSSatellites.SatsInView = double(fread(fid, 1, 'int8'));
    GPSSatellites.PRN = double(fread(fid, 16, 'int8'));
    GPSSatellites.Elevation = double(fread(fid, 16, 'float32'));
    GPSSatellites.Azimuth = double(fread(fid, 16, 'float32'));
    GPSSatellites.SNR = double(fread(fid, 16, 'int8'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [GPSTime] = ReadGPSTimeObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        GPSTime.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    GPSTime.timestamp = timestamp;
    GPSTime.Month = double(fread(fid, 1, 'int8'));
    GPSTime.Day = double(fread(fid, 1, 'int8'));
    GPSTime.Year = double(fread(fid, 1, 'int16'));
    GPSTime.Hour = double(fread(fid, 1, 'int8'));
    GPSTime.Minute = double(fread(fid, 1, 'int8'));
    GPSTime.Second = double(fread(fid, 1, 'int8'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [GuidanceSettings] = ReadGuidanceSettingsObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        GuidanceSettings.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    GuidanceSettings.timestamp = timestamp;
    GuidanceSettings.GuidanceMode = double(fread(fid, 1, 'uint8'));
    GuidanceSettings.MaxGroundspeed = double(fread(fid, 1, 'int32'));
    GuidanceSettings.GroundVelocityP = double(fread(fid, 1, 'float32'));
    GuidanceSettings.MaxVerticalSpeed = double(fread(fid, 1, 'int32'));
    GuidanceSettings.VertVelocityP = double(fread(fid, 1, 'float32'));
    GuidanceSettings.VelP = double(fread(fid, 1, 'float32'));
    GuidanceSettings.VelI = double(fread(fid, 1, 'float32'));
    GuidanceSettings.VelD = double(fread(fid, 1, 'float32'));
    GuidanceSettings.DownP = double(fread(fid, 1, 'float32'));
    GuidanceSettings.DownI = double(fread(fid, 1, 'float32'));
    GuidanceSettings.DownD = double(fread(fid, 1, 'float32'));
    GuidanceSettings.MaxVelIntegral = double(fread(fid, 1, 'float32'));
    GuidanceSettings.MaxThrottleIntegral = double(fread(fid, 1, 'float32'));
    GuidanceSettings.VelUpdatePeriod = double(fread(fid, 1, 'int32'));
    GuidanceSettings.VelPIDUpdatePeriod = double(fread(fid, 1, 'int32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [HomeLocation] = ReadHomeLocationObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        HomeLocation.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    HomeLocation.timestamp = timestamp;
    HomeLocation.Set = double(fread(fid, 1, 'uint8'));
    HomeLocation.Latitude = double(fread(fid, 1, 'int32'));
    HomeLocation.Longitude = double(fread(fid, 1, 'int32'));
    HomeLocation.Altitude = double(fread(fid, 1, 'float32'));
    HomeLocation.ECEF = double(fread(fid, 3, 'int32'));
    HomeLocation.RNE = double(fread(fid, 9, 'float32'));
    HomeLocation.Be = double(fread(fid, 3, 'float32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [I2CStats] = ReadI2CStatsObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        I2CStats.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    I2CStats.timestamp = timestamp;
    I2CStats.event_errors = double(fread(fid, 1, 'uint16'));
    I2CStats.fsm_errors = double(fread(fid, 1, 'uint16'));
    I2CStats.irq_errors = double(fread(fid, 1, 'uint16'));
    I2CStats.last_error_type = double(fread(fid, 1, 'uint8'));
    I2CStats.evirq_log = double(fread(fid, 5, 'uint32'));
    I2CStats.erirq_log = double(fread(fid, 5, 'uint32'));
    I2CStats.event_log = double(fread(fid, 5, 'uint8'));
    I2CStats.state_log = double(fread(fid, 5, 'uint8'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [ManualControlCommand] = ReadManualControlCommandObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        ManualControlCommand.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    ManualControlCommand.timestamp = timestamp;
    ManualControlCommand.Connected = double(fread(fid, 1, 'uint8'));
    ManualControlCommand.Armed = double(fread(fid, 1, 'uint8'));
    ManualControlCommand.Roll = double(fread(fid, 1, 'float32'));
    ManualControlCommand.Pitch = double(fread(fid, 1, 'float32'));
    ManualControlCommand.Yaw = double(fread(fid, 1, 'float32'));
    ManualControlCommand.Throttle = double(fread(fid, 1, 'float32'));
    ManualControlCommand.FlightMode = double(fread(fid, 1, 'uint8'));
    ManualControlCommand.StabilizationSettings = double(fread(fid, 3, 'uint8'));
    ManualControlCommand.Accessory1 = double(fread(fid, 1, 'float32'));
    ManualControlCommand.Accessory2 = double(fread(fid, 1, 'float32'));
    ManualControlCommand.Accessory3 = double(fread(fid, 1, 'float32'));
    ManualControlCommand.Channel = double(fread(fid, 8, 'int16'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [ManualControlSettings] = ReadManualControlSettingsObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        ManualControlSettings.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    ManualControlSettings.timestamp = timestamp;
    ManualControlSettings.InputMode = double(fread(fid, 1, 'uint8'));
    ManualControlSettings.Roll = double(fread(fid, 1, 'uint8'));
    ManualControlSettings.Pitch = double(fread(fid, 1, 'uint8'));
    ManualControlSettings.Yaw = double(fread(fid, 1, 'uint8'));
    ManualControlSettings.Throttle = double(fread(fid, 1, 'uint8'));
    ManualControlSettings.FlightMode = double(fread(fid, 1, 'uint8'));
    ManualControlSettings.Accessory1 = double(fread(fid, 1, 'uint8'));
    ManualControlSettings.Accessory2 = double(fread(fid, 1, 'uint8'));
    ManualControlSettings.Accessory3 = double(fread(fid, 1, 'uint8'));
    ManualControlSettings.Pos1StabilizationSettings = double(fread(fid, 3, 'uint8'));
    ManualControlSettings.Pos2StabilizationSettings = double(fread(fid, 3, 'uint8'));
    ManualControlSettings.Pos3StabilizationSettings = double(fread(fid, 3, 'uint8'));
    ManualControlSettings.Pos1FlightMode = double(fread(fid, 1, 'uint8'));
    ManualControlSettings.Pos2FlightMode = double(fread(fid, 1, 'uint8'));
    ManualControlSettings.Pos3FlightMode = double(fread(fid, 1, 'uint8'));
    ManualControlSettings.ChannelMax = double(fread(fid, 8, 'int16'));
    ManualControlSettings.ChannelNeutral = double(fread(fid, 8, 'int16'));
    ManualControlSettings.ChannelMin = double(fread(fid, 8, 'int16'));
    ManualControlSettings.ArmedTimeout = double(fread(fid, 1, 'uint16'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [MixerSettings] = ReadMixerSettingsObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        MixerSettings.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    MixerSettings.timestamp = timestamp;
    MixerSettings.MaxAccel = double(fread(fid, 1, 'float32'));
    MixerSettings.FeedForward = double(fread(fid, 1, 'float32'));
    MixerSettings.AccelTime = double(fread(fid, 1, 'float32'));
    MixerSettings.DecelTime = double(fread(fid, 1, 'float32'));
    MixerSettings.ThrottleCurve1 = double(fread(fid, 5, 'float32'));
    MixerSettings.ThrottleCurve2 = double(fread(fid, 5, 'float32'));
    MixerSettings.Mixer1Type = double(fread(fid, 1, 'uint8'));
    MixerSettings.Mixer1Vector = double(fread(fid, 5, 'int8'));
    MixerSettings.Mixer2Type = double(fread(fid, 1, 'uint8'));
    MixerSettings.Mixer2Vector = double(fread(fid, 5, 'int8'));
    MixerSettings.Mixer3Type = double(fread(fid, 1, 'uint8'));
    MixerSettings.Mixer3Vector = double(fread(fid, 5, 'int8'));
    MixerSettings.Mixer4Type = double(fread(fid, 1, 'uint8'));
    MixerSettings.Mixer4Vector = double(fread(fid, 5, 'int8'));
    MixerSettings.Mixer5Type = double(fread(fid, 1, 'uint8'));
    MixerSettings.Mixer5Vector = double(fread(fid, 5, 'int8'));
    MixerSettings.Mixer6Type = double(fread(fid, 1, 'uint8'));
    MixerSettings.Mixer6Vector = double(fread(fid, 5, 'int8'));
    MixerSettings.Mixer7Type = double(fread(fid, 1, 'uint8'));
    MixerSettings.Mixer7Vector = double(fread(fid, 5, 'int8'));
    MixerSettings.Mixer8Type = double(fread(fid, 1, 'uint8'));
    MixerSettings.Mixer8Vector = double(fread(fid, 5, 'int8'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [MixerStatus] = ReadMixerStatusObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        MixerStatus.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    MixerStatus.timestamp = timestamp;
    MixerStatus.Mixer1 = double(fread(fid, 1, 'float32'));
    MixerStatus.Mixer2 = double(fread(fid, 1, 'float32'));
    MixerStatus.Mixer3 = double(fread(fid, 1, 'float32'));
    MixerStatus.Mixer4 = double(fread(fid, 1, 'float32'));
    MixerStatus.Mixer5 = double(fread(fid, 1, 'float32'));
    MixerStatus.Mixer6 = double(fread(fid, 1, 'float32'));
    MixerStatus.Mixer7 = double(fread(fid, 1, 'float32'));
    MixerStatus.Mixer8 = double(fread(fid, 1, 'float32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [ObjectPersistence] = ReadObjectPersistenceObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        ObjectPersistence.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    ObjectPersistence.timestamp = timestamp;
    ObjectPersistence.Operation = double(fread(fid, 1, 'uint8'));
    ObjectPersistence.Selection = double(fread(fid, 1, 'uint8'));
    ObjectPersistence.ObjectID = double(fread(fid, 1, 'uint32'));
    ObjectPersistence.InstanceID = double(fread(fid, 1, 'uint32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [PipXtremeModemSettings] = ReadPipXtremeModemSettingsObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        PipXtremeModemSettings.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    PipXtremeModemSettings.timestamp = timestamp;
    PipXtremeModemSettings.Mode = double(fread(fid, 1, 'uint8'));
    PipXtremeModemSettings.Serial_Baudrate = double(fread(fid, 1, 'uint8'));
    PipXtremeModemSettings.Frequency_Calibration = double(fread(fid, 1, 'uint8'));
    PipXtremeModemSettings.Frequency_Min = double(fread(fid, 1, 'uint32'));
    PipXtremeModemSettings.Frequency_Max = double(fread(fid, 1, 'uint32'));
    PipXtremeModemSettings.Frequency = double(fread(fid, 1, 'uint32'));
    PipXtremeModemSettings.Max_RF_Bandwidth = double(fread(fid, 1, 'uint8'));
    PipXtremeModemSettings.Max_Tx_Power = double(fread(fid, 1, 'uint8'));
    PipXtremeModemSettings.Tx_Data_Wait = double(fread(fid, 1, 'uint8'));
    PipXtremeModemSettings.AES_Encryption = double(fread(fid, 1, 'uint8'));
    PipXtremeModemSettings.AES_EncryptionKey = double(fread(fid, 16, 'uint8'));
    PipXtremeModemSettings.Paired_Serial_Number = double(fread(fid, 1, 'uint32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [PipXtremeModemStatus] = ReadPipXtremeModemStatusObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        PipXtremeModemStatus.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    PipXtremeModemStatus.timestamp = timestamp;
    PipXtremeModemStatus.Firmware_Version_Major = double(fread(fid, 1, 'uint8'));
    PipXtremeModemStatus.Firmware_Version_Minor = double(fread(fid, 1, 'uint8'));
    PipXtremeModemStatus.Serial_Number = double(fread(fid, 1, 'uint32'));
    PipXtremeModemStatus.Up_Time = double(fread(fid, 1, 'uint32'));
    PipXtremeModemStatus.Frequency = double(fread(fid, 1, 'uint32'));
    PipXtremeModemStatus.RF_Bandwidth = double(fread(fid, 1, 'uint32'));
    PipXtremeModemStatus.Tx_Power = double(fread(fid, 1, 'int8'));
    PipXtremeModemStatus.State = double(fread(fid, 1, 'uint8'));
    PipXtremeModemStatus.Tx_Retry = double(fread(fid, 1, 'uint16'));
    PipXtremeModemStatus.Tx_Data_Rate = double(fread(fid, 1, 'uint32'));
    PipXtremeModemStatus.Rx_Data_Rate = double(fread(fid, 1, 'uint32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [PositionActual] = ReadPositionActualObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        PositionActual.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    PositionActual.timestamp = timestamp;
    PositionActual.North = double(fread(fid, 1, 'int32'));
    PositionActual.East = double(fread(fid, 1, 'int32'));
    PositionActual.Down = double(fread(fid, 1, 'int32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [PositionDesired] = ReadPositionDesiredObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        PositionDesired.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    PositionDesired.timestamp = timestamp;
    PositionDesired.North = double(fread(fid, 1, 'int32'));
    PositionDesired.East = double(fread(fid, 1, 'int32'));
    PositionDesired.Down = double(fread(fid, 1, 'int32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [RateDesired] = ReadRateDesiredObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        RateDesired.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    RateDesired.timestamp = timestamp;
    RateDesired.Roll = double(fread(fid, 1, 'float32'));
    RateDesired.Pitch = double(fread(fid, 1, 'float32'));
    RateDesired.Yaw = double(fread(fid, 1, 'float32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [StabilizationSettings] = ReadStabilizationSettingsObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        StabilizationSettings.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    StabilizationSettings.timestamp = timestamp;
    StabilizationSettings.RollMax = double(fread(fid, 1, 'uint8'));
    StabilizationSettings.PitchMax = double(fread(fid, 1, 'uint8'));
    StabilizationSettings.YawMax = double(fread(fid, 1, 'uint8'));
    StabilizationSettings.ManualRate = double(fread(fid, 3, 'float32'));
    StabilizationSettings.MaximumRate = double(fread(fid, 3, 'float32'));
    StabilizationSettings.RollRatePI = double(fread(fid, 3, 'float32'));
    StabilizationSettings.PitchRatePI = double(fread(fid, 3, 'float32'));
    StabilizationSettings.YawRatePI = double(fread(fid, 3, 'float32'));
    StabilizationSettings.RollPI = double(fread(fid, 3, 'float32'));
    StabilizationSettings.PitchPI = double(fread(fid, 3, 'float32'));
    StabilizationSettings.YawPI = double(fread(fid, 3, 'float32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [SystemAlarms] = ReadSystemAlarmsObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        SystemAlarms.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    SystemAlarms.timestamp = timestamp;
    SystemAlarms.Alarm = double(fread(fid, 14, 'uint8'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [SystemSettings] = ReadSystemSettingsObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        SystemSettings.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    SystemSettings.timestamp = timestamp;
    SystemSettings.AirframeType = double(fread(fid, 1, 'uint8'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [SystemStats] = ReadSystemStatsObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        SystemStats.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    SystemStats.timestamp = timestamp;
    SystemStats.FlightTime = double(fread(fid, 1, 'uint32'));
    SystemStats.HeapRemaining = double(fread(fid, 1, 'uint16'));
    SystemStats.CPULoad = double(fread(fid, 1, 'uint8'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [TaskInfo] = ReadTaskInfoObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        TaskInfo.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    TaskInfo.timestamp = timestamp;
    TaskInfo.StackRemaining = double(fread(fid, 13, 'uint16'));
    TaskInfo.Running = double(fread(fid, 13, 'uint8'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [TelemetrySettings] = ReadTelemetrySettingsObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        TelemetrySettings.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    TelemetrySettings.timestamp = timestamp;
    TelemetrySettings.Speed = double(fread(fid, 1, 'uint8'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [VelocityActual] = ReadVelocityActualObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        VelocityActual.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    VelocityActual.timestamp = timestamp;
    VelocityActual.North = double(fread(fid, 1, 'int32'));
    VelocityActual.East = double(fread(fid, 1, 'int32'));
    VelocityActual.Down = double(fread(fid, 1, 'int32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [VelocityDesired] = ReadVelocityDesiredObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        VelocityDesired.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    VelocityDesired.timestamp = timestamp;
    VelocityDesired.North = double(fread(fid, 1, 'int32'));
    VelocityDesired.East = double(fread(fid, 1, 'int32'));
    VelocityDesired.Down = double(fread(fid, 1, 'int32'));
    % read CRC
    fread(fid, 1, 'uint8');
end

function [WatchdogStatus] = ReadWatchdogStatusObject(fid, timestamp)
    if 1
        headerSize = 8;
    else
        WatchdogStatus.instanceID = fread(fid, 1, 'uint16');
        headerSize = 10;
    end

    WatchdogStatus.timestamp = timestamp;
    WatchdogStatus.BootupFlags = double(fread(fid, 1, 'uint16'));
    WatchdogStatus.ActiveFlags = double(fread(fid, 1, 'uint16'));
    % read CRC
    fread(fid, 1, 'uint8');
end




