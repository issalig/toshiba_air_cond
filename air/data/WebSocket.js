var rainbowEnable = false;
var power = false;
var save = false;
 
function updateStatus(data)
{
   document.getElementById('serial').innerHTML = data;//data.toString(); 
}

var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);

connection.onopen = function () {
    connection.send('Connect ' + new Date());
    document.getElementById('connection').textContent = "open";
    
   (function scheduleRequest() {
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
      setTimeout(scheduleRequest, 500); //ask for status every 500 ms
   })();
};

connection.onerror = function (error) {
    console.log('WebSocket Error ', error);
    document.getElementById('connection').textContent = "error";
};

connection.onmessage = function (e) {  //data from server
    console.log('Server: ', e.data);
    //updateStatus(e.data);
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

  if (json.save=='1') save = true; else save=false;
  if (json.power=='1') power = true; else power=false;
  
  //document.getElementById('save').textContent = json.save;
  //document.getElementById('heat').textContent = json.heat;
  //document.getElementById('cool').textContent = json.cool;
  document.getElementById('temp').textContent = json.temp;
  document.getElementById('sensor_temp').textContent = json.sensor_temp;
  //document.getElementById('fan').textContent = json.fan;  
  //document.getElementById('mode').textContent = json.mode;
  //document.getElementById('power').textContent = json.power;
  
  //add last cmd if not the same
  textlen = document.getElementById('last_cmd').textContent.length;
  cmdlen = json.last_cmd.length;
  if (document.getElementById('last_cmd').textContent[textlen-2] != json.last_cmd[cmdlen-2])
  document.getElementById('last_cmd').innerHTML += '<br>' + json.last_cmd;
  
  //document.getElementById('timer_time').textContent = json.timer_time;
  document.getElementById('timer_pending').textContent = json.timer_pending;


  //reset color
  document.getElementById('fan_low').style.backgroundColor = '#999';
  document.getElementById('fan_medium').style.backgroundColor = '#999';
  document.getElementById('fan_high').style.backgroundColor = '#999';
  document.getElementById('fan_auto').style.backgroundColor = '#999';
 
  document.getElementById('mode_fan').style.backgroundColor = '#999';
  document.getElementById('mode_dry').style.backgroundColor = '#999';
  document.getElementById('mode_cool').style.backgroundColor = '#999';
  document.getElementById('mode_heat').style.backgroundColor = '#999';
  
  document.getElementById('power_on').style.backgroundColor = '#999';
  document.getElementById('power_off').style.backgroundColor = '#999';

  document.getElementById('save_button').style.backgroundColor = '#999';

  //set color
  if (json.fan == 'LOW') { document.getElementById('fan_low').style.backgroundColor = '#00878F';}
  else if (json.fan == 'MED') { document.getElementById('fan_medium').style.backgroundColor = '#00878F'; }
  else if (json.fan == 'HIGH') { document.getElementById('fan_high').style.backgroundColor = '#00878F'; }
  else if (json.fan == 'AUTO') { document.getElementById('fan_auto').style.backgroundColor = '#00878F'; }       
  
  if (json.mode == 'DRY') { document.getElementById('mode_dry').style.backgroundColor = '#00878F';}
  else if (json.mode == 'COOL') { document.getElementById('mode_cool').style.backgroundColor = '#00878F'; }
  else if (json.mode == 'FAN') { document.getElementById('mode_fan').style.backgroundColor = '#00878F'; }
  else if (json.mode == 'HEAT') { document.getElementById('mode_heat').style.backgroundColor = '#00878F'; }
  
  if (json.power == '1') { document.getElementById('power_on').style.backgroundColor = '#00878F';} 
  else { document.getElementById('power_off').style.backgroundColor = '#00878F';} 
  
  if (json.save == '1') { document.getElementById('save_button').style.backgroundColor = '#00878F';} 
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

function tempp() {
    let data = {
       id : "temp",
       temp : "1"
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);
}

function tempm() {
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
    let data = {
       id : "timer",
       timer_mode : document.getElementById('timer_on').value+document.getElementById('timer_off').value,
       timer_time : document.getElementById('timer_range').value
    }
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json); 
}

function sendRGB() {
    var r = document.getElementById('r').value**2/1023;
    var g = document.getElementById('g').value**2/1023;
    var b = document.getElementById('b').value**2/1023;
    
    var rgb = r << 20 | g << 10 | b;
    var rgbstr = '#'+ rgb.toString(16);    
    console.log('RGB: ' + rgbstr); 
    connection.send(rgbstr);
}

function rainbowEffect(){
    rainbowEnable = ! rainbowEnable;
    if(rainbowEnable){
        connection.send("R");
        document.getElementById('rainbow').style.backgroundColor = '#00878F';
        document.getElementById('r').className = 'disabled';
        document.getElementById('g').className = 'disabled';
        document.getElementById('b').className = 'disabled';
        document.getElementById('r').disabled = true;
        document.getElementById('g').disabled = true;
        document.getElementById('b').disabled = true;
    } else {
        connection.send("N");
        document.getElementById('rainbow').style.backgroundColor = '#999';
        document.getElementById('r').className = 'enabled';
        document.getElementById('g').className = 'enabled';
        document.getElementById('b').className = 'enabled';
        document.getElementById('r').disabled = false;
        document.getElementById('g').disabled = false;
        document.getElementById('b').disabled = false;
        sendRGB();
    }  
}
