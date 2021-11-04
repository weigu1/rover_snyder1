// here comes the html css and javascript magic
String html_css_js = R"=====(
<!--http://twiggle-web-design.com/tutorials/Custom-Vertical-Input-Range-CSS3.html-->
<!DOCTYPE html>
<html>
<head>    
  <title>creative-lab.lu</title>
  <script>
    var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);
    connection.onopen = function () {
      connection.send('Connect ' + new Date());
    };
    connection.onerror = function (error) {
      console.log('WebSocket Error', error);
    };
    connection.onmessage = function (e) {
      console.log('Server: ', e.data);
    };
    connection.onclose = function(){
      console.log('WebSocket connection closed');
    };
    function send_data() {
      var h = parseInt(document.getElementById('h').value).toString(16);
      var v = parseInt(document.getElementById('v').value).toString(16);      
      if (h.length == 1) {
        h = "00" + h;
      }
      else if (h.length == 2) {
        h = '0' + h;
      } 
      if (v.length == 1) {
        v = "00" + v;
      } 
      else if (v.length == 2) {
        v = '0' + v;
      } 

      var payload = '#'+h+v;
      console.log('payload: ' + payload);
      connection.send(payload);
    }
  </script>
  <style>
    html {
      background-color: #FFeaea;
      color: #555;
      height: 100%;
      padding: 0px;
      margin: 0px;
      overflow:hidden;
      text-align:center;
    }
    body {
      font-family: Arial, Calibri;
      width: 100%;
      margin: 0px;
      background-color: Black;
      padding: 0px;
      height: 100%;
    }    

    input[type='range'].range {
      position: fixed;
      cursor: grabbing;
      width: 50% !important;
      -webkit-appearance: none;
      width:50%;
      border: 0px;
      background-color: #e6e6e6;
      background-image: -webkit-gradient(linear, 0% 0%, 0% 100%, from(#e6e6e6), to(#d2d2d2));
      background-image: -webkit-linear-gradient(right, #222222, #AAAAAA,#222222);
      background-image: -moz-linear-gradient(right, #222222, #AAAAAA,#222222);
      background-image: -ms-linear-gradient(right, #222222, #AAAAAA,#222222);
      background-image: -o-linear-gradient(right, #222222, #AAAAAA,#222222);
    }
    input[type='range'].range:focus {
      border: 0 !imporant;
      outline: none !important;
    }
    input[type='range'].range::-webkit-slider-thumb  {
      -webkit-appearance: none;
      width: 100px;
      height: 100px; background-color: #555;
      background-image: -webkit-gradient(linear, 0% 0%, 0% 100%, from(#4DFFFF), to(#00CCFF));
      background-image: -webkit-linear-gradient(right,#000000, #00cc00,#00cc00,#000000);
      background-image: -moz-linear-gradient(right,#000000, #00cc00,#00cc00,#000000);
      background-image: -ms-linear-gradient(right,#000000, #00cc00,#00cc00,#000000);
      background-image: -o-linear-gradient(right,#000000, #00cc00,#00cc00,#000000);
    }
    input[type='range'].round {
      -webkit-border-radius: 20px;
      -moz-border-radius: 20px;
      border-radius: 20px;
    }
    input[type='range'].round::-webkit-slider-thumb  {
      -webkit-border-radius: 20px;
      -moz-border-radius: 20px;
      -o-border-radius: 20px;
    }
    .horizontal-lowest-first {   
      -webkit-transform:rotate(0deg);       
      -moz-transform:rotate(0deg);
      -o-transform:rotate(0deg);
      -ms-transform:rotate(0deg);
      transform:rotate(0deg);
    }
    .vertical-heighest-first {
      -webkit-transform:rotate(270deg);       
      -moz-transform:rotate(270deg);
      -o-transform:rotate(270deg);
      -ms-transform:rotate(270deg);
      transform:rotate(270deg);
    }
    .slider_h {
      top: 40%;
      left: 2%;
    }
    .slider_v {
      top: 40%;
      left: 55%;
    }
  .noselect {
    -webkit-touch-callout: none; /* iOS Safari */
    -webkit-user-select: none; /* Safari */
    -khtml-user-select: none; /* Konqueror HTML */
    -moz-user-select: none; /* Firefox */
    -ms-user-select: none; /* Internet Explorer/Edge */
     user-select: none; /* Non-prefixed version, currently supported by Chrome and Opera */
  }
  </style>
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=0"/>
</head>
<body>
  <h1>snyder by creative-lab.lu</h1>
  <input id='h' class='range horizontal-lowest-first round slider_h' type='range' oninput='send_data();' min='0' max='1023' step='1' value='512' >
  <input id='v' class='range vertical-heighest-first round slider_v' type='range' oninput='send_data();' min='0' max='1023' step='1' value='512' >
</body>
</html>
)=====";
