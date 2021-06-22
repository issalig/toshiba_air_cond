//https://coolors.co/ffbe0b-fb5607-ff006e-8338ec-3a86ff
//https://circuits4you.com/2019/01/25/esp8266-dht11-humidity-temperature-data-logging/
//https://circuits4you.com/2019/01/11/esp8266-data-logging-with-real-time-graphs/
//https://nagix.github.io/chartjs-plugin-datasource/samples/json-dataset.html

var power = false;
var save = false;
var debug_cmd = false;
var chart;
var ac_sensor_values=[];
var ac_target_values=[];
var dht_t_values=[25,27,30,25];
var dht_h_values=[60,70,80,90];
var timeStamp = [0,1,2,3];
var chart_obj;


var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);

connection.onopen = function () {
    connection.send('Connect ' + new Date());
    document.getElementById('connection').textContent = "open";
    
   (function scheduleRequestStatus() {
      //send json            
	  var today = new Date();
      var curr_time = today.getHours() + ":" + today.getMinutes() + ":" + today.getSeconds();
        let data = {
			id: "status",
			value: curr_time
		}
		let json = JSON.stringify(data);
		connection.send(json);
      //end send json

      setTimeout(scheduleRequestStatus, 2000); //ask for status 
   })();

   (function scheduleRequestTimeSeries() {	  
		getTimeseries();
      setTimeout(scheduleRequestTimeSeries, 60*1000); //ask for time series every minute
   })();

};

connection.onerror = function (error) {
    console.log('WebSocket Error ', error);
    document.getElementById('connection').textContent = "error";
};

connection.onmessage = function (e) {  //data from server
    console.log('Server: ', e.data);
    processData(e.data);
};

connection.onclose = function(e){
    console.log('WebSocket connection closed');
    console.log('Reconnect will be attempted in 1 second.', e.reason);
    document.getElementById('connection').textContent = "closed";
    connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);

    //setTimeout(function() {
	//	connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);
	//	console.log('Reconnecting.');
	//	}, 1000);
};

//function sendData()
//{
  //let ledNumber = document.getElementById('ledNumber');
  //let ledStatus = document.querySelector('input[name="status"]:checked');
 
  //let data = {
     //command : "Set",
     //id: ledNumber.value,
     //status: ledStatus.value
  //}
  //let json = JSON.stringify(data);
 
  //connection.send(json);
//}
 
//function getData()
//{
  //let data = {
     //command : "Get"
  //}
  //let json = JSON.stringify(data);
 
  //connection.send(json);
//}

function processData(data)
{
  let json = JSON.parse(data); 
  console.log(json);

  if (json.id=="status") parseStatus(json);
  if (json.id=="timeseries") parseTimeSeries(json);
  if (json.id=="debug") parseDebug(json);
}



function parseStatus(json)
{
  button_pressed_color ='#002F7A';//#8338EC';// '#BCC6CC';
  button_released_color='#99C0FF';

  if (json.save=='1') save = true; else save=false;
  if (json.power=='1') power = true; else power=false;
  
  //document.getElementById('save').textContent = json.save;
  //document.getElementById('heat').textContent = json.heat;
  //document.getElementById('cool').textContent = json.cool;
  document.getElementById('temp').textContent = json.temp;
  document.getElementById('sensor_temp').textContent = json.sensor_temp;
  document.getElementById('dht_temp').textContent = json.dht_temp;
  document.getElementById('dht_hum').textContent = json.dht_hum;
  document.getElementById('bmp_temp').textContent = json.bmp_temp;
  document.getElementById('bmp_press').textContent = json.bmp_press;

  //document.getElementById('sampling_text').value = json.sampling;

  //document.getElementById('fan').textContent = json.fan;  
  //document.getElementById('mode').textContent = json.mode;
  //document.getElementById('power').textContent = json.power;
  
  document.getElementById('decode_errors').textContent = json.decode_errors;

  document.getElementById('heap').textContent = json.heap;

  debug_cmd=document.getElementById('check_cmd').checked;
  
  cmdlen=0;
  if (debug_cmd){
	if (json.last_cmd){ //if not null or empty
		textlen = document.getElementById('last_cmd').textContent.length;
		cmdlen = json.last_cmd.length;
		//add last cmd that it is not the same
		if (document.getElementById('last_cmd').textContent[textlen-4] != json.last_cmd[cmdlen-4])  //-2
			document.getElementById('last_cmd').innerHTML +=  json.last_cmd;
    }
  } else document.getElementById('last_cmd').innerHTML = json.last_cmd;

  debug_rx=document.getElementById('check_rx').checked;

  cmdlen=0;


	if (debug_rx){
if (json.rx_data){ //if not null or empty	
		textlen = document.getElementById('rx_data').textContent.length; //rx_data
		cmdlen = json.rx_data.length;
		//add last rx_data that it is not the same
		//if (document.getElementById('rx_data').textContent[textlen-4] != json.rx_data[cmdlen-4])  //-2
		if (cmdlen > 1)
			document.getElementById('rx_data').innerHTML += '<br>' + json.rx_data;
    }
  } else document.getElementById('rx_data').innerHTML = json.rx_data;

	//if (json.rx_data)
		//document.getElementById('rx_data').innerHTML += '<br>' + json.rx_data;


  //document.getElementById('timer_time').textContent = json.timer_time;
  document.getElementById('timer_pending').textContent = json.timer_pending;


  //reset color
  document.getElementById('fan_low').style.backgroundColor = button_released_color;
  document.getElementById('fan_medium').style.backgroundColor = button_released_color;
  document.getElementById('fan_high').style.backgroundColor = button_released_color;
  document.getElementById('fan_auto').style.backgroundColor = button_released_color;
 
  document.getElementById('mode_fan').style.backgroundColor = button_released_color;
  document.getElementById('mode_dry').style.backgroundColor = button_released_color;
  document.getElementById('mode_cool').style.backgroundColor = button_released_color;
  document.getElementById('mode_heat').style.backgroundColor = button_released_color;
  
  document.getElementById('preheat').style.backgroundColor = button_released_color;
  
  document.getElementById('power_on').style.backgroundColor = button_released_color;
  document.getElementById('power_off').style.backgroundColor = button_released_color;

  document.getElementById('save_button').style.backgroundColor = button_released_color;

  document.getElementById('sw_timer_button').style.backgroundColor = button_released_color;
  

  //set color
  if (json.fan == 'LOW') { document.getElementById('fan_low').style.backgroundColor = button_pressed_color;}
  else if (json.fan == 'MED') { document.getElementById('fan_medium').style.backgroundColor = button_pressed_color; }
  else if (json.fan == 'HIGH') { document.getElementById('fan_high').style.backgroundColor = button_pressed_color; }
  else if (json.fan == 'AUTO') { document.getElementById('fan_auto').style.backgroundColor = button_pressed_color; }       
  
  if (json.mode == 'DRY') { document.getElementById('mode_dry').style.backgroundColor = button_pressed_color;}
  else if (json.mode == 'COOL') { document.getElementById('mode_cool').style.backgroundColor = button_pressed_color; }
  else if (json.mode == 'FAN') { document.getElementById('mode_fan').style.backgroundColor = button_pressed_color; }
  else if (json.mode == 'HEAT') { document.getElementById('mode_heat').style.backgroundColor = button_pressed_color; }
  
  if (json.power == '1') { document.getElementById('power_on').style.backgroundColor = button_pressed_color;} 
  else { document.getElementById('power_off').style.backgroundColor = button_pressed_color;} 
  
  if (json.save == '1') { document.getElementById('save_button').style.backgroundColor = button_pressed_color;}
  
  if (json.timer_enabled =='1')   { document.getElementById('sw_timer_button').style.backgroundColor = button_pressed_color;}
  
  if (json.preheat == '1') { document.getElementById('preheat').style.backgroundColor = button_pressed_color;} 
  else { document.getElementById('preheat').style.backgroundColor = button_released_color;} 
  
  //document.getElementById('indoor_room_temp').textContent = json.indoor_room_temp;
  document.getElementById('indoor_ta').textContent = json.indoor_ta;
  document.getElementById('indoor_tcj').textContent = json.indoor_tcj;
  document.getElementById('indoor_tc').textContent = json.indoor_tc;
  //document.getElementById('indoor_filter_time').textContent = json.indoor_filter_time;
  document.getElementById('outdoor_te').textContent = json.outdoor_te;
  document.getElementById('outdoor_to').textContent = json.outdoor_to;
  //document.getElementById('outdoor_td').textContent = json.outdoor_td;
  //document.getElementById('outdoor_ts').textContent = json.outdoor_ts;
  //document.getElementById('outdoor_ths').textContent = json.outdoor_ths;
  document.getElementById('outdoor_current').textContent = json.outdoor_current;
  //document.getElementById('outdoor_cumhour').textContent = json.outdoor_cumhour;
  
  document.getElementById('indoor_fan_speed').textContent = json.indoor_fan_speed;
  //document.getElementById('indoor_fan_run_time').textContent = json.indoor_fan_run_time;
  
  //document.getElementById('outdoor_tl').textContent = json.outdoor_tl;
  //document.getElementById('outdoor_comp_freq').textContent = json.outdoor_comp_freq;
  //document.getElementById('outdoor_lower_fan_speed').textContent = json.outdoor_lower_fan_speed;
  //document.getElementById('outdoor_upper_fan_speed').textContent = json.outdoor_upper_fan_speed;
  
  var d = new Date(parseInt(json.boot_time*1000));
  document.getElementById('boot_time').innerHTML = "On "+ d.toLocaleTimeString() + " " + d.toLocaleDateString()
  
}

function power_on() {
    let data = {
       id : "power",
       value : "on"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
    
    document.getElementById('power_on').style.backgroundColor = button_pressed_color;
}

function power_off() {
    let data = {
       id : "power",
       value : "off"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
    
    document.getElementById('power_off').style.backgroundColor = button_pressed_color;
}

function save_toggle() {
	var save_value;
	if (save) save_value = "0"; else save_value = "1";	
    let data = {
       id : "save",
       value : save_value
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
}

function mode_fan() {
    let data = {
       id : "mode",
       value : "fan"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
    
    document.getElementById('mode_fan').style.backgroundColor = button_pressed_color;
}

function mode_dry() {
    let data = {
       id : "mode",
       value : "dry"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
    
    document.getElementById('mode_dry').style.backgroundColor = button_pressed_color;
}

function mode_cool() {
    let data = {
       id : "mode",
       value : "cool"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
    
    document.getElementById('mode_cool').style.backgroundColor = button_pressed_color;
}

function mode_heat() {
    let data = {
       id : "mode",
       value : "heat"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
    
    document.getElementById('mode_heat').style.backgroundColor = button_pressed_color;
}

function fan_low() {
    let data = {
       id : "fan",
       value : "low"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
    
    document.getElementById('fan_low').style.backgroundColor = button_pressed_color;
}

function fan_medium() {
    let data = {
       id : "fan",
       value : "medium"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
    
    document.getElementById('fan_medium').style.backgroundColor = button_pressed_color;
}

function fan_high() {
    let data = {
       id : "fan",
       value : "high"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
    
    document.getElementById('fan_high').style.backgroundColor = button_pressed_color;
}

function fan_auto() {
    let data = {
       id : "fan",
       value : "auto"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
    
    document.getElementById('fan_auto').style.backgroundColor = button_pressed_color;
}

function temp_plus() {
    let data = {
       id : "temp",
       temp : "1"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
}

function temp_minus() {
    let data = {
       id : "temp",
       temp : "0"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json); 
}

function timer_changed() {
	document.getElementById('timer_time').textContent  = document.getElementById('timer_range').value;
}

function set_sw_air_timer(){
	
	document.getElementById('timer_time').textContent  = document.getElementById('timer_range').value;
	if (document.getElementById('timer_off').checked) val = 0; else val=1;
	
    let data = {
       id : "timer",
       timer_mode : val,
       timer_time : document.getElementById('timer_range').value
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json); 
}

function timer_hw_cancel() {
    let data = {
       id : "timer_hw",
       value : "cancel",
       time: "1"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
    
    document.getElementById('timer_hw_repeat_off').style.backgroundColor = button_released_color;
    document.getElementById('timer_hw_off').style.backgroundColor = button_released_color;
    document.getElementById('timer_hw_on').style.backgroundColor = button_released_color;
}

function timer_hw_off() {
    let data = {
       id : "timer_hw",
       value : "off",
       time: "1"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
    
    document.getElementById('timer_hw_repeat_off').style.backgroundColor = button_released_color;
    document.getElementById('timer_hw_off').style.backgroundColor = button_pressed_color;
    document.getElementById('timer_hw_on').style.backgroundColor = button_released_color;
}

function timer_hw_repeat_off() {
    let data = {
       id : "timer_hw",
       value : "repeat_off",
       time: "1"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
    
    document.getElementById('timer_hw_repeat_off').style.backgroundColor = button_pressed_color;
    document.getElementById('timer_hw_off').style.backgroundColor = button_released_color;
    document.getElementById('timer_hw_on').style.backgroundColor = button_released_color;
}

function timer_hw_on() {
    let data = {
       id : "timer_hw",
       value : "on",
       time: "1"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
    
    document.getElementById('timer_hw_repeat_off').style.backgroundColor = button_released_color;
    document.getElementById('timer_hw_off').style.backgroundColor = button_released_color;
    document.getElementById('timer_hw_on').style.backgroundColor = button_pressed_color;
}

//time series request
function getTimeseries() {
    let data = {
       id : "timeseries",
       value : "all"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
}

//time series request
function getTimeseriesKeyExperimental(key) {
    let data = {
       id : "timeseriesexperimental",
       value : key
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
}



function set_sampling() {
    let data = {
       id : "sampling",
       value : parseInt(document.getElementById('sampling_text').value)
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
}

//https://coolors.co/9c12f3-47a8bd-f5e663-ffad69-9c3848
//255, 173, 105
//156, 56, 72
//245, 230, 99
//71, 168, 189 

function showGraph()
{
    var ctx = document.getElementById("temp_chart").getContext('2d');
var config={
        type: 'line',
        data: {
            labels: [0],  //Bottom Labeling
			//if not labels set xaxis to linear https://github.com/chartjs/Chart.js/issues/5494
            datasets: [{
                label: 'AC Sensor',
                yAxisID: 'temperature',
				fill: false,  //Try with true
                backgroundColor: 'rgba(156, 56, 72, 1)', //Dot marker color
                borderColor: 'rgba(156, 56, 72, 1)', //Graph Line Color
                data: [0],
            },/*{
                label: 'AC Target',
                yAxisID: 'temperature',
				fill: false,  //Try with true
                backgroundColor: 'rgba( 71, 168, 189 , 1)', //Dot marker color
                borderColor: 'rgba( 71, 168, 189, 1)', //Graph Line Color
                data: [0],			
            },*/

            {
                label: 'Room Temperature',
				yAxisID: 'temperature',
                fill: false,  //Try with true
                backgroundColor: 'rgba( 243, 156, 18 , 1)', //Dot marker color
                borderColor: 'rgba( 243, 156, 18 , 1)', //Graph Line Color
                data: [0],
            },{
                label: 'Room Humidity',
                yAxisID: 'humidity',
				fill: false,  //Try with true
                backgroundColor: 'rgba(156, 18, 243 , 1)', //Dot marker color
                borderColor: 'rgba(156, 18, 243 , 1)', //Graph Line Color
                data: [0],
	     },/*{
                label: 'BMP Temperature',
				yAxisID: 'temperature',
                fill: false,  //Try with true
                backgroundColor: 'rgba( 156, 243, 18 , 1)', //Dot marker color
                borderColor: 'rgba( 156, 243, 18 , 1)', //Graph Line Color
                data: [0],            
            },*/{
                label: 'BMP Pressure',
				yAxisID: 'pressure',
                fill: false,  //Try with true
                backgroundColor: 'rgba( 243, 18, 243 , 1)', //Dot marker color
                borderColor: 'rgba( 243, 18, 243 , 1)', //Graph Line Color
                data: [0],
            },
            {
                label: 'TOut',
                yAxisID: 'temperature',
				fill: false,  //Try with true
                backgroundColor: 'rgba(156, 72, 56, 1)', //Dot marker color
                borderColor: 'rgba(156, 72, 56, 1)', //Graph Line Color
                data: [0],
            },
            
            
            ],
        },
        options: {
            title: {
                    display: true,
                    text: "Air conditioning data"
                },
            maintainAspectRatio: false,
            elements: {
            line: {
                    tension: 0.5 //Smoothening (Curved) of data lines
                }
            },
            scales: {
                    yAxes: [{
						id: 'temperature',
						gridLines: {
							drawOnChartArea: false
						},
                        ticks: {
                            //beginAtZero:true,    
							//suggestedMin: 10,
							//suggestedMax: 40
							type: 'logarithmic',
							min: 5,
							max: 34
                        },
						scaleLabel: {
							display: true,
							labelString: 'Temperature ºC'
						},
                    },{
						id: 'humidity',
						gridLines: {
							drawOnChartArea: false
						},
                        ticks: {
                            //beginAtZero:true,
							type: 'logarithmic',    
							min: 35,
							max: 80
                        },
						position: 'right',
						scaleLabel: {
							display: true,
							labelString: 'Humidity %'
						},
					},{
						id: 'pressure',
						gridLines: {
							drawOnChartArea: false
						},
                        ticks: {
                            //beginAtZero:true,
							type: 'logarithmic',    
							min: 950, //mb
							max: 1050
                        },
						position: 'right',
						scaleLabel: {
							display: true,
							labelString: 'Pressure (mb)'
						},
					}],
                    xAxes: [{
						scaleLabel: {
							display: true,
							labelString: 'Time'
						}
					}],
					/*xAxes: [{
						scaleLabel: {
							display: true,
							labelString: 'Time'
						},
						distribution: 'linear',
						type: 'time',
						time: {
							parser: 'MM/DD/YYYY HH:mm',
							// round: 'day'
							tooltipFormat: 'll HH:mm'
						},
						time: {
							parser: 'MM/DD/YYYY HH:mm',
							tooltipFormat: 'll HH:mm',
							unit: 'day',
							unitStepSize: 1,
							displayFormats: {
								'day': 'MM/DD/YYYY'}
						}
					}], */
           },
            animation: {
                duration: 0,
			}
         }
         }

    chart_obj = new Chart(ctx, config);           
}
 
//check for time labeling
//https://www.chartjs.org/samples/latest/scales/time/line.html

/*
setInterval(function() {
  // Call a function repetatively with 5 Second interval
  getData();
}, 5000); //5000mSeconds update rate
*/ 

function getMinNoZero(data){
	min=9999999
	datalen=data.length;
	for(i=0;i<datalen;i++){
		if ((data[i] < min) && (data[i] !=0 )){
			min=data[i];
		}
	}
	
    if (min==9999999) min=0 //all zeroes
    
	return min;
}

//sometimes esp sends unexpected big values, just filter them
function getMaxWithLimits(data){
	max=-1000
	datalen=data.length;
	for(i=0;i<datalen;i++){
		if ((data[i] > max) && (data[i] < 1000)){
			max=data[i];
		}
	}
	
    if (max==-1000) max=0
    
	return max;
}


function parseTimeSeries(json){
	datalen=json.n;
    console.log(datalen);
	
	if (!chart_obj) showGraph();


    //https://stackoverflow.com/questions/49360165/chart-js-update-function-chart-labels-data-will-not-update-the-chart
    //update chart
    
    if (json.val == "timestamp"){
		 timeStamp=[]; 

		for(i=0;i<datalen;i++){
			timeStamp.push(moment(json.timestamp[i]*1000).format('HH:mm'));//(i);		
		} 

		chart_obj.data.labels=timeStamp;//json.timestamp;//
	}
	
	if (json.val == "ac_sensor_t")
		chart_obj.data.datasets[0].data=json.ac_sensor_t;
	if (json.val == "dht_t")
		chart_obj.data.datasets[1].data=json.dht_t;
	if (json.val == "dht_h")			
		chart_obj.data.datasets[2].data=json.dht_h;
	if (json.val == "bmp_p")    
		chart_obj.data.datasets[3].data=json.bmp_p;
	if (json.val == "to")    		
		chart_obj.data.datasets[4].data=json.to;
    //chart_obj.data.datasets[3].backgroundColor= 'rgba('+Math.floor()*255+','+Math.floor()*255+','+Math.floor()*255+', 1)';
	//chart_obj.data.datasets[3].borderColor= chart_obj.data.datasets[3].backgroundColor;

    //window.
    chart_obj.update();
    
    //compute min/max
	if (json.val == "dht_t"){
		document.getElementById('dht_temp_min').textContent = getMinNoZero(json.dht_t);//Math.min(...json.dht_t);
		document.getElementById('dht_temp_max').textContent = getMaxWithLimits(json.dht_t);//Math.max(...json.dht_t);
    }
    if (json.val == "dht_h"){        
		document.getElementById('dht_hum_min').textContent  = getMinNoZero(json.dht_h);//Math.min(...json.dht_h);
		document.getElementById('dht_hum_max').textContent  = getMaxWithLimits(json.dht_h);//Math.max(...json.dht_h);
	}
	if (json.val == "bmp_p"){
		document.getElementById('bmp_pres_min').textContent = getMinNoZero(json.bmp_p);//Math.min(...json.bmp_p);
		document.getElementById('bmp_pres_max').textContent = getMaxWithLimits(json.bmp_p);//Math.max(...json.bmp_p);
	}
}

function parseTimeSeriesOLD(json){
	datalen=json.dht_t.length;
    console.log(datalen);
	
	if (!chart_obj) showGraph();

    timeStamp=[]; 

    for(i=0;i<datalen;i++){
		timeStamp.push(moment(json.timestamp[i]*1000).format('HH:mm'));//(i);		
    } 

    //https://stackoverflow.com/questions/49360165/chart-js-update-function-chart-labels-data-will-not-update-the-chart
    //update chart
	chart_obj.data.labels=timeStamp;//json.timestamp;//
	chart_obj.data.datasets[0].data=json.ac_sensor_t;
//    chart_obj.data.datasets[1].data=json.ac_target_t;
	chart_obj.data.datasets[1].data=json.dht_t;
    chart_obj.data.datasets[2].data=json.dht_h;
    //chart_obj.data.datasets[4].data=json.bmp_t;
    chart_obj.data.datasets[3].data=json.bmp_p;
    chart_obj.data.datasets[4].data=json.to;
    //chart_obj.data.datasets[3].backgroundColor= 'rgba('+Math.floor()*255+','+Math.floor()*255+','+Math.floor()*255+', 1)';
	//chart_obj.data.datasets[3].borderColor= chart_obj.data.datasets[3].backgroundColor;

    //window.
    chart_obj.update();
    
    //compute min/max
    document.getElementById('dht_temp_min').textContent = getMinNoZero(json.dht_t);//Math.min(...json.dht_t);
    document.getElementById('dht_temp_max').textContent = getMaxWithLimits(json.dht_t);//Math.max(...json.dht_t);
    document.getElementById('dht_hum_min').textContent  = getMinNoZero(json.dht_h);//Math.min(...json.dht_h);
    document.getElementById('dht_hum_max').textContent  = getMaxWithLimits(json.dht_h);//Math.max(...json.dht_h);
    document.getElementById('bmp_pres_min').textContent = getMinNoZero(json.bmp_p);//Math.min(...json.bmp_p);
    document.getElementById('bmp_pres_max').textContent = getMaxWithLimits(json.bmp_p);//Math.max(...json.bmp_p);
}


function parseTimeSeriesxx(json){
	//datalen=json.dht_t.length;
    //console.log(datalen);


  debug_rx=document.getElementById('check_rx').checked;

  cmdlen=0;


	if (debug_rx){
if (json.rx_data){ //if not null or empty	
		textlen = document.getElementById('rx_data').textContent.length;
		cmdlen = json.rx_data.length;
		//add last rx_data is not the same
		if (document.getElementById('rx_data').textContent[textlen-2] != json.rx_data[cmdlen-2])
			document.getElementById('rx_data').innerHTML += '<br>DBG ' + json.rx_data;
    }
  } else document.getElementById('rx_data').innerHTML = json.rx_data;


	
}
//On Page load show graphs
window.onload = function() {
    getTimeseries();
	showGraph();
};


