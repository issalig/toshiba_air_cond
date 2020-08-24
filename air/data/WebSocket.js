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

      setTimeout(scheduleRequestStatus, 750); //ask for status every 500 ms
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
  //document.getElementById('fan').textContent = json.fan;  
  //document.getElementById('mode').textContent = json.mode;
  //document.getElementById('power').textContent = json.power;
  
document.getElementById('decode_errors').textContent = json.decode_errors;


  debug_cmd=document.getElementById('check_cmd').checked;
  
  cmdlen=0;
  if (debug_cmd){
	if (json.last_cmd){ //if not null or empty
		textlen = document.getElementById('last_cmd').textContent.length;
		cmdlen = json.last_cmd.length;
		//add last cmd is not the same
		if (document.getElementById('last_cmd').textContent[textlen-2] != json.last_cmd[cmdlen-2])
			document.getElementById('last_cmd').innerHTML += '<br>' + json.last_cmd;
    }
  } else document.getElementById('last_cmd').innerHTML = json.last_cmd;

  debug_rx=document.getElementById('check_rx').checked;

  cmdlen=0;


	if (debug_rx){
if (json.rx_data){ //if not null or empty	
		textlen = document.getElementById('rx_data').textContent.length;
		cmdlen = json.rx_data.length;
		//add last rx_data is not the same
		if (document.getElementById('rx_data').textContent[textlen-2] != json.rx_data[cmdlen-2])
			document.getElementById('rx_data').innerHTML += '<br>RX ' + json.rx_data;
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
  
  document.getElementById('power_on').style.backgroundColor = button_released_color;
  document.getElementById('power_off').style.backgroundColor = button_released_color;

  document.getElementById('save_button').style.backgroundColor = button_released_color;

  document.getElementById('timer_button').style.backgroundColor = button_released_color;
  

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
  
  if (json.timer_enabled =='1')   { document.getElementById('timer_button').style.backgroundColor = button_pressed_color;}
  
}

function power_on() {
    let data = {
       id : "power",
       value : "on"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
}

function power_off() {
    let data = {
       id : "power",
       value : "off"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
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
}

function mode_dry() {
    let data = {
       id : "mode",
       value : "dry"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
}

function mode_cool() {
    let data = {
       id : "mode",
       value : "cool"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
}

function mode_heat() {
    let data = {
       id : "mode",
       value : "heat"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
}

function fan_low() {
    let data = {
       id : "fan",
       value : "low"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
}

function fan_medium() {
    let data = {
       id : "fan",
       value : "medium"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
}

function fan_high() {
    let data = {
       id : "fan",
       value : "high"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
}

function fan_auto() {
    let data = {
       id : "fan",
       value : "auto"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
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

function setACTimer(){
	
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

function getTimeseries() {
    let data = {
       id : "timeseries",
       value : "all"
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
            },{
                label: 'AC Target',
                yAxisID: 'temperature',
				fill: false,  //Try with true
                backgroundColor: 'rgba( 71, 168, 189 , 1)', //Dot marker color
                borderColor: 'rgba( 71, 168, 189, 1)', //Graph Line Color
                data: [0],
            },{
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
            }],
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
							min: 20,
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
							min: 60,
							max: 75
                        },
						position: 'right',
						scaleLabel: {
							display: true,
							labelString: 'Humidity %'
						},
					}],
                    xAxes: [{
						scaleLabel: {
							display: true,
							labelString: 'Time'
						}
					}],
					
           },
            animation: {
                duration: 0,
			}
         }
         }

    chart_obj = new Chart(ctx, config);           
}
 
/*
setInterval(function() {
  // Call a function repetatively with 5 Second interval
  getData();
}, 5000); //5000mSeconds update rate
*/ 


function parseTimeSeries(json){
	datalen=json.dht_t.length;
    console.log(datalen);
	
	if (!chart_obj) showGraph();

    timeStamp=[]; 

    for(i=0;i<datalen;i++){
		timeStamp.push(i);		
    } 

    //https://stackoverflow.com/questions/49360165/chart-js-update-function-chart-labels-data-will-not-update-the-chart
    //update chart
	chart_obj.data.labels=timeStamp;
	chart_obj.data.datasets[0].data=json.ac_sensor_t;
    chart_obj.data.datasets[1].data=json.ac_target_t;
	chart_obj.data.datasets[2].data=json.dht_t;
    chart_obj.data.datasets[3].data=json.dht_h;
    //chart_obj.data.datasets[3].backgroundColor= 'rgba('+Math.floor()*255+','+Math.floor()*255+','+Math.floor()*255+', 1)';
	//chart_obj.data.datasets[3].borderColor= chart_obj.data.datasets[3].backgroundColor;

    //window.
    chart_obj.update();

     /*
	//Update Data Table
    var table = document.getElementById("dataTable");
    var row = table.insertRow(1); //Add after headings
    var cell1 = row.insertCell(0);
    var cell2 = row.insertCell(1);
    var cell3 = row.insertCell(2);
    var cell4 = row.insertCell(3);
    cell1.innerHTML = time;
    cell2.innerHTML = ac_sensor_values[i];
    cell3.innerHTML = ac_target_values[i];
    cell4.innerHTML = dht_t_values[i];
	cell5.innerHTML = dht_h_values[i];
*/
}
//On Page load show graphs
window.onload = function() {
    getTimeseries();
	showGraph();
};




//chart
//window.onload = function () {

//var chart = new CanvasJS.Chart("chartContainer", {
	//animationEnabled: true,
	//title:{
		//text: "Temperature / Humidity"
	//},
	//axisX: {
		//valueFormatString: "DD MMM,YY"
	//},
	//axisY: {
		//title: "Temperature (in °C)",
		//suffix: " °C"
	//},
	//legend:{
		//cursor: "pointer",
		//fontSize: 16,
		//itemclick: toggleDataSeries
	//},
	//toolTip:{
		//shared: true
	//},
	//data: [{
		//name: "Myrtle Beach",
		//type: "spline",
		//yValueFormatString: "#0.## °C",
		//showInLegend: true,
		//dataPoints: [
			//{ x: new Date(2017,6,24), y: 31 },
			//{ x: new Date(2017,6,25), y: 31 },
			//{ x: new Date(2017,6,26), y: 29 },
			//{ x: new Date(2017,6,27), y: 29 },
			//{ x: new Date(2017,6,28), y: 31 },
			//{ x: new Date(2017,6,29), y: 30 },
			//{ x: new Date(2017,6,30), y: 29 }
		//]
	//},
	//{
		//name: "Martha Vineyard",
		//type: "spline",
		//yValueFormatString: "#0.## °C",
		//showInLegend: true,
		//dataPoints: [
			//{ x: new Date(2017,6,24), y: 20.5 },
			//{ x: new Date(2017,6,25), y: 20.5 },
			//{ x: new Date(2017,6,26), y: 25 },
			//{ x: new Date(2017,6,27), y: 25 },
			//{ x: new Date(2017,6,28), y: 25 },
			//{ x: new Date(2017,6,29), y: 25 },
			//{ x: new Date(2017,6,30), y: 25 }
		//]
	//},
	//{
		//name: "Nantucket",
		//type: "spline",
		//yValueFormatString: "#0.## °C",
		//showInLegend: true,
		//dataPoints: [
			//{ x: new Date(2017,6,24), y: 22 },
			//{ x: new Date(2017,6,25), y: 19 },
			//{ x: new Date(2017,6,26), y: 23 },
			//{ x: new Date(2017,6,27), y: 24 },
			//{ x: new Date(2017,6,28), y: 24 },
			//{ x: new Date(2017,6,29), y: 23 },
			//{ x: new Date(2017,6,30), y: 23 }
		//]
	//}]
//});
//chart.render();

//function toggleDataSeries(e){
	//if (typeof(e.dataSeries.visible) === "undefined" || e.dataSeries.visible) {
		//e.dataSeries.visible = false;
	//}
	//else{
		//e.dataSeries.visible = true;
	//}
	//chart.render();
//}

//}
