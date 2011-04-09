package org.openpilot.uavtalk;
/**
 * class with static functions to parse values from the UAVTalk protocol 
 * 
 * @author ligi ( aka: Marcus Bueschleb | mail: ligi at ligi dot de )
 */
public class ValueParser {

	/**
	 * parse a int value from 4 bytes of some array
	 * 
	 * @param offset - where to start in the array
	 * @param arr - the array
	 * @return - the calculated value
	 */
	
	public final static int parse_int_from_arr_4(int offset,byte[] arr) {
		return 	(
				 ((arr[offset+3]&0xFF)<<24) |
				 ((arr[offset+2]&0xFF)<<16) |
				 ((arr[offset+1]&0xFF)<<8)  |
				   arr[offset+0]&0xFF 
				);
	}

	public final static int parse_int_from_arr_4_2(int offset,byte[] arr) {
		return 	(
				 ((arr[offset+3])<<24) |
				 ((arr[offset+2])<<16) |
				 ((arr[offset+1])<<8)  |
				   arr[offset+0] 
				);
	}
}
