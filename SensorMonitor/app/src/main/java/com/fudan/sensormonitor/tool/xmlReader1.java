package com.fudan.sensormonitor.tool;

import com.fudan.sensormonitor.model.*;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;


public class xmlReader1 {
	static final int INTERVAL = 24;
	
	public static HashMap readFromXML(String path, String type) {
		int[] data = new int[INTERVAL];
		HashMap<Integer,int[]> appList = new HashMap<Integer,int[]>();
		
		HashMap ret = new HashMap();
		
		DocumentBuilderFactory factory = null;
		DocumentBuilder builder = null;
		Document document = null;
		InputStream inputStream = null;
		@SuppressWarnings("unused")
		Element camera, microphone, gps, other;
		factory = DocumentBuilderFactory.newInstance();
		
		String format = "EEE MMM dd HH:mm:ss yyyy";
		try {
			// loading file
			builder = factory.newDocumentBuilder();
			File file = new File( path);
			inputStream = new FileInputStream(file);
			document = builder.parse(inputStream);
			
			// find the root element
			Element root = document.getDocumentElement();
			Element ele = (Element)root.getElementsByTagName(type.toLowerCase()).item(0);
			
			if (type.equalsIgnoreCase("gps")) {
				HashMap map = gpsUsage(ele, format);
				data = (int[]) map.get("list");
				appList = (HashMap<Integer, int[]>) map.get("appList");
			}else if(type.equalsIgnoreCase("camera")){
				HashMap map = cameraUsage(ele, format);
				data = (int[]) map.get("list");
				appList = (HashMap<Integer, int[]>) map.get("appList");
			}else if(type.equalsIgnoreCase("microphone")){
				HashMap map = microphoneUsage(ele, format);
				data = (int[]) map.get("list");
				appList = (HashMap<Integer, int[]>) map.get("appList");
			}else if(type.equalsIgnoreCase("standard sensors")){
				HashMap map = standardUsage(ele, format);
				data = (int[]) map.get("list");
				appList = (HashMap<Integer, int[]>) map.get("appList");
			}
		
		} catch (IOException e) {
			e.printStackTrace();
		} catch (SAXException e) {
			e.printStackTrace();
		} catch (ParserConfigurationException e) {
			e.printStackTrace();
		} 
		ret.put("data", data);
		ret.put("appList", appList);
		return ret;
	}

	//calculate the time different between the record time and the system time for now
	public static long dateDiff(String recTime, String format) {
		String sysTime = new SimpleDateFormat(format).format(new Date());
		long diff = 0;
		System.out.println("recTime =======  "+recTime);
		System.out.println("sysTime =======  "+sysTime);
		SimpleDateFormat sdf = new SimpleDateFormat(format);
		SimpleDateFormat sdf1 = new SimpleDateFormat(format,Locale.ENGLISH);
		long nd = 1000 * 24 * 60 * 60;// 一天的毫秒数
		long nh = 1000 * 60 * 60;// 一小时的毫秒数
		long nm = 1000 * 60;// 一分钟的毫秒数
		long ns = 1000;// 一秒钟的毫秒数
		long day = 0;
		long hour = 0;
		
		 try {     
	            diff = sdf.parse(sysTime).getTime() - sdf1.parse(recTime).getTime();     
	            day = diff / nd;// 计算差多少天     
	            hour = diff % nd / nh + day * 24;// 计算差多少小时     
	        } catch (ParseException e) {     
	            // TODO Auto-generated catch block     
	            e.printStackTrace();     
	        }     
		return hour;
	}

	public static HashMap  gpsUsage(Element ele, String format){
	HashMap ret = new HashMap();
	int[] list = new int[INTERVAL];
	HashMap<Integer, int[]> appList = new HashMap<Integer, int[]>();
	int[] temp;
	
	NodeList access = ele.getElementsByTagName("access");
	for(int i=0; i<access.getLength(); i++){
		GpsData obj;
		Node acc= access.item(i);
		String time_start="",time_end="";
		int uid=0,usage=0;
		for(Node node = acc.getFirstChild(); node!=null;node=node.getNextSibling()){
			if(node.getNodeType()==Node.ELEMENT_NODE){
				if(node.getNodeName().equalsIgnoreCase("time_start")){
					time_start=node.getFirstChild().getNodeValue();
					//System.out.println("nodename ==="+node.getNodeName());
				 	//System.out.println("time_start ==="+time_start);
				}else if(node.getNodeName().equalsIgnoreCase("time_end")){
					time_end=node.getFirstChild().getNodeValue();
					//System.out.println("nodename ==="+node.getNodeName());
				 	//System.out.println("time_end ==="+time_end);
				}else if(node.getNodeName().equalsIgnoreCase("uid")){
					uid=Integer.parseInt(node.getFirstChild().getNodeValue());
					//System.out.println("nodename ==="+node.getNodeName());
				 	//System.out.println("uid ==="+uid);
				}else if(node.getNodeName().equalsIgnoreCase("usage")){
					usage=Integer.parseInt(node.getFirstChild().getNodeValue());
					//System.out.println("nodename ==="+node.getNodeName());
				 	//System.out.println("usage ==="+usage);
				}
			}
		}
		obj = new GpsData(time_start,time_end,usage,uid);
		long diff = dateDiff(obj.getTime_end(),format);
		if(diff<24){
			list[INTERVAL-1-(int) (diff/(24/INTERVAL))]+=obj.getUsage();
			if(appList.get(uid)==null){
				temp = new int[INTERVAL];
				temp[INTERVAL-1-(int) (diff/(24/INTERVAL))] = obj.getUsage();
				appList.put(obj.getUid(), temp);
			}else{
				temp = appList.get(uid);
				temp[INTERVAL-1-(int) (diff/(24/INTERVAL))]+=obj.getUsage();
				appList.put(obj.getUid(), temp);
			}
		}
		
	}
	ret.put("list", list);
	ret.put("appList", appList);
	return ret;
}

	public static HashMap  cameraUsage(Element ele, String format){
		HashMap ret = new HashMap();
		int[] list = new int[INTERVAL];
		HashMap<Integer, int[]> appList = new HashMap<Integer, int[]>();
		int[] temp;

		NodeList record = ele.getElementsByTagName("record");
		for(int i=0; i<record.getLength(); i++){
			CameraData obj;
			Node rec= record.item(i);
			String time_start="",time_end="";
			int uid=0,usage=0;
			for(Node node = rec.getFirstChild(); node!=null;node=node.getNextSibling()){
				if(node.getNodeType()==Node.ELEMENT_NODE){
					if(node.getNodeName().equalsIgnoreCase("time_start")){
						time_start=node.getFirstChild().getNodeValue();
						//System.out.println("nodename ==="+node.getNodeName());
						System.out.println("time_start ==="+time_start);
					}else if(node.getNodeName().equalsIgnoreCase("time_end")){
						time_end=node.getFirstChild().getNodeValue();
						//System.out.println("nodename ==="+node.getNodeName());
						System.out.println("time_end ==="+time_end);
					}else if(node.getNodeName().equalsIgnoreCase("uid")){
						uid=Integer.parseInt(node.getFirstChild().getNodeValue());
						//System.out.println("nodename ==="+node.getNodeName());
						System.out.println("uid ==="+uid);
					}else if(node.getNodeName().equalsIgnoreCase("usage_video")){
						usage=Integer.parseInt(node.getFirstChild().getNodeValue());
						//System.out.println("nodename ==="+node.getNodeName());
						System.out.println("usage ==="+usage);
					}
				}
			}
			obj = new CameraData(time_start,time_end,usage,uid);
			long diff = dateDiff(obj.getTime_end(),format);
			if(diff<24){
				list[INTERVAL-1-(int) (diff/(24/INTERVAL))]+=obj.getUsage();
				if(appList.get(uid)==null){
					temp = new int[INTERVAL];
					temp[INTERVAL-1-(int) (diff/(24/INTERVAL))] = obj.getUsage();
					appList.put(obj.getUid(), temp);
				}else{
					temp = appList.get(uid);
					temp[INTERVAL-1-(int) (diff/(24/INTERVAL))]+=obj.getUsage();
					appList.put(obj.getUid(), temp);
				}
			}

		}
		ret.put("list", list);
		ret.put("appList", appList);
		return ret;
	}

	public static HashMap  microphoneUsage(Element ele, String format){
		HashMap ret = new HashMap();
		int[] list = new int[INTERVAL];
		HashMap<Integer, int[]> appList = new HashMap<Integer, int[]>();
		int[] temp;

		NodeList record = ele.getElementsByTagName("record");
		for(int i=0; i<record.getLength(); i++){
			MicrophoneData obj;
			Node acc= record.item(i);
			String time_start="",time_end="";
			int uid=0,usage=0;
			for(Node node = acc.getFirstChild(); node!=null;node=node.getNextSibling()){
				if(node.getNodeType()==Node.ELEMENT_NODE){
					if(node.getNodeName().equalsIgnoreCase("time_start")){
						time_start=node.getFirstChild().getNodeValue();
						//System.out.println("nodename ==="+node.getNodeName());
						System.out.println("time_start ==="+time_start);
					}else if(node.getNodeName().equalsIgnoreCase("time_end")){
						time_end=node.getFirstChild().getNodeValue();
						//System.out.println("nodename ==="+node.getNodeName());
						System.out.println("time_end ==="+time_end);
					}else if(node.getNodeName().equalsIgnoreCase("uid")){
						uid=Integer.parseInt(node.getFirstChild().getNodeValue());
						//System.out.println("nodename ==="+node.getNodeName());
						System.out.println("uid ==="+uid);
					}else if(node.getNodeName().equalsIgnoreCase("usage")){
						usage=Integer.parseInt(node.getFirstChild().getNodeValue())/1048576;
						//System.out.println("nodename ==="+node.getNodeName());
						System.out.println("usage ==="+usage);
					}
				}
			}
			obj = new MicrophoneData(time_start,time_end,usage,uid);
			long diff = dateDiff(obj.getTime_end(),format);
			if(diff<24){
				list[INTERVAL-1-(int) (diff/(24/INTERVAL))]+=obj.getUsage();
				if(appList.get(uid)==null){
					temp = new int[INTERVAL];
					temp[INTERVAL-1-(int) (diff/(24/INTERVAL))] = obj.getUsage();
					appList.put(obj.getUid(), temp);
				}else{
					temp = appList.get(uid);
					temp[INTERVAL-1-(int) (diff/(24/INTERVAL))]+=obj.getUsage();
					appList.put(obj.getUid(), temp);
				}
			}

		}
		ret.put("list", list);
		ret.put("appList", appList);
		return ret;
	}

	public static HashMap  standardUsage(Element ele, String format){
		HashMap ret = new HashMap();
		int[] list = new int[INTERVAL];
		HashMap<Integer, int[]> appList = new HashMap<Integer, int[]>();
		int[] temp;

		NodeList access = ele.getElementsByTagName("access");
		for(int i=0; i<access.getLength(); i++){
			GpsData obj;
			Node acc= access.item(i);
			String time_start="",time_end="";
			int uid=0,usage=0;
			for(Node node = acc.getFirstChild(); node!=null;node=node.getNextSibling()){
				if(node.getNodeType()==Node.ELEMENT_NODE){
					if(node.getNodeName().equalsIgnoreCase("time_start")){
						time_start=node.getFirstChild().getNodeValue();
						//System.out.println("nodename ==="+node.getNodeName());
						System.out.println("time_start ==="+time_start);
					}else if(node.getNodeName().equalsIgnoreCase("time_end")){
						time_end=node.getFirstChild().getNodeValue();
						//System.out.println("nodename ==="+node.getNodeName());
						System.out.println("time_end ==="+time_end);
					}else if(node.getNodeName().equalsIgnoreCase("uid")){
						uid=Integer.parseInt(node.getFirstChild().getNodeValue());
						//System.out.println("nodename ==="+node.getNodeName());
						System.out.println("uid ==="+uid);
					}else if(node.getNodeName().equalsIgnoreCase("usage")){
						usage=Integer.parseInt(node.getFirstChild().getNodeValue());
						//System.out.println("nodename ==="+node.getNodeName());
						System.out.println("usage ==="+usage);
					}
				}
			}
			obj = new GpsData(time_start,time_end,usage,uid);
			long diff = dateDiff(obj.getTime_end(),format);
			if(diff<24){
				list[INTERVAL-1-(int) (diff/(24/INTERVAL))]+=obj.getUsage();
				if(appList.get(uid)==null){
					temp = new int[12];
					temp[INTERVAL-1-(int) (diff/2)] = obj.getUsage();
					appList.put(obj.getUid(), temp);
				}else{
					temp = appList.get(uid);
					temp[INTERVAL-1-(int) (diff/2)]+=obj.getUsage();
					appList.put(obj.getUid(), temp);
				}
			}

		}
		ret.put("list", list);
		ret.put("appList", appList);
		return ret;
	}
}