var rainbowEnable = false;
var power = false;
 
function updateStatus(data)
{
   document.getElementById('serial').innerHTML = data;//"cacasion"; //data.toString(); 
}

var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);
connection.onopen = function () {
    connection.send('Connect ' + new Date());
    
       // Ejemplo 1, peticion desde cliente
   (function scheduleRequest() {
      //connection.send("L");
      //send json
        let data = {
			id: "status"
		}
		let json = JSON.stringify(data);
		connection.send(json);
      //end send json
      setTimeout(scheduleRequest, 500);
   })();
};
connection.onerror = function (error) {
    console.log('WebSocket Error ', error);
};
connection.onmessage = function (e) {  //data from server
    console.log('Server: ', e.data);
    //updateStatus(e.data);
    processData(e.data);
};
connection.onclose = function(e){
    console.log('WebSocket connection closed');
    console.log('Reconnect will be attempted in 1 second.', e.reason);
    setTimeout(function() {
      connection();
    }, 1000);
};

function sendData()
{
  let ledNumber = document.getElementById('ledNumber');
  let ledStatus = document.querySelector('input[name="status"]:checked');
 
  let data = {
     command : "Set",
     id: ledNumber.value,
     status: ledStatus.value
  }
  let json = JSON.stringify(data);
 
  connection.send(json);
}
 
function getData()
{
  let data = {
     command : "Get"
  }
  let json = JSON.stringify(data);
 
  connection.send(json);
}

function processData(data)
{
  let json = JSON.parse(data); 
  console.log(json);
  
  document.getElementById('save').textContent = json.save;
  document.getElementById('heat').textContent = json.heat;
  document.getElementById('cold').textContent = json.cold;
  document.getElementById('temp').textContent = json.temp;
  document.getElementById('sensor_temp').textContent = json.sensor_temp;
  document.getElementById('fan').textContent = json.fan;
  //document.getElementById('fan_str').textContent = json.fan_str;
  document.getElementById('mode').textContent = json.mode;
  //document.getElementById('mode_str').textContent = json.mode_str;
  document.getElementById('power').textContent = json.power;
  document.getElementById('last_cmd').textContent = json.last_cmd;
  
  //document.getElementById('timer_time').textContent = json.timer_time;
  document.getElementById('timer_pending').textContent = json.timer_pending;

}

function powerb() {
	power = !power;
	var str = 'W';
    console.log('POWER: ' + str); 
    connection.send(str);
    if (power)
		document.getElementById('powerb').style.backgroundColor = '#00878F';      
    else        
        document.getElementById('powerb').style.backgroundColor = '#999';
    
    let data = {
       id : "power",
       mode : power
    }
     //document.getElementById('power').textContent = json.power;
     
    let json = JSON.stringify(data);
 
    connection.send(json);
    console.log('Send: ' + json);         
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

function mode_cold() {
    let data = {
       id : "mode",
       value : "cool"
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
