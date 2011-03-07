package org.openpilot.uavtalk;

public class Telemetry {
	
	private TelemetryStats stats;
    public class TelemetryStats {
        public int txBytes;
        public int rxBytes;
        public int txObjectBytes;
        public int rxObjectBytes;
        public int rxObjects;
        public int txObjects;
        public int txErrors;
        public int rxErrors;
        public int txRetries;
    } ;
    
    public TelemetryStats getStats() {
    	return stats;
    }    
    
    public void resetStats() {
    	stats = new TelemetryStats();
    }
}
