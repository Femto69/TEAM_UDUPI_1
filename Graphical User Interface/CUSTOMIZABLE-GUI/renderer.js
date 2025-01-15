const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');
const { Chart } = require('chart.js/auto');

let serialPort;
let gyroChart;
let accelChart;
let tempChart;
let altitudeChart;
let pressureChart;
let humidityChart; // New chart for humidity
let lastUpdateTime = 0;
let dataLog = [];
function initSerialPort() {
  SerialPort.list().then((ports) => {
    const portSelect = document.getElementById('port-select');
    portSelect.innerHTML = ''; // Clear existing options
    ports.forEach((port) => {
      const option = document.createElement('option');
      option.value = port.path;
      option.textContent = port.path;
      portSelect.appendChild(option);
    });
  }).catch(err => {
    console.error('Error listing ports:', err);
    showError('Failed to list serial ports. Please check your connection and try again.');
  });
}

function toggleConnection() {
  const connectButton = document.getElementById('connect-button');
  if (serialPort && serialPort.isOpen) {
    disconnectFromPort();
  } else {
    connectToPort();
  }
}

function connectToPort() {
  const portPath = document.getElementById('port-select').value;
  const connectButton = document.getElementById('connect-button');

  serialPort = new SerialPort({ path: portPath, baudRate: 115200 }, function (err) {
    if (err) {
      console.error('Error opening port:', err);
      showError(`Failed to open port ${portPath}. ${err.message}`);
      return;
    }
    console.log('Port opened successfully');
    connectButton.textContent = 'Disconnect';
    showError(''); // Clear any previous error messages
  });

  const parser = serialPort.pipe(new ReadlineParser({ delimiter: '\n' }));

  parser.on('data', (data) => {
    const values = data.split(',').map(Number);
    if (values.length === 11) { // Updated to expect 11 values (including altitude, pressure, and humidity)
      const now = Date.now();
      if (now - lastUpdateTime > 100) { // Update every 100ms
        updateGyroChart(values.slice(0, 3));
        updateAccelChart(values.slice(3, 6));
        updateTempChart(values.slice(6, 8));
        updateAltitudeChart([values[8]]);
        updatePressureChart([values[9]]);
        updateHumidityChart([values[10]]); // New humidity data
        updateDataDisplay(values);
        dataLog.push({ time: now, values: values });
        lastUpdateTime = now;
      }
    }
  });
  serialPort.on('error', function(err) {
    console.error('Serial port error:', err);
    showError(`Serial port error: ${err.message}`);
  });

  serialPort.on('close', function() {
    console.log('Serial port closed');
    connectButton.textContent = 'Connect';
    showError('Serial port was closed. Please check your connection.');
  });
}

function disconnectFromPort() {
  if (serialPort) {
    serialPort.close((err) => {
      if (err) {
        console.error('Error closing port:', err);
        showError(`Failed to close port. ${err.message}`);
      } else {
        console.log('Port closed successfully');
        document.getElementById('connect-button').textContent = 'Connect';
        showError(''); // Clear any error messages
      }
    });
  }
}

function initCharts() {
  const gyroCtx = document.getElementById('gyro-chart').getContext('2d');
  const accelCtx = document.getElementById('accel-chart').getContext('2d');
  const tempCtx = document.getElementById('temp-chart').getContext('2d');
  const altitudeCtx = document.getElementById('altitude-chart').getContext('2d');
  const pressureCtx = document.getElementById('pressure-chart').getContext('2d');
  const humidityCtx = document.getElementById('humidity-chart').getContext('2d'); // New context for humidity chart

  gyroChart = new Chart(gyroCtx, createChartConfig('Gyroscope', ['X', 'Y', 'Z']));
  accelChart = new Chart(accelCtx, createChartConfig('Accelerometer', ['X', 'Y', 'Z']));
  tempChart = new Chart(tempCtx, createChartConfig('Temperature', ['Temperature', 'BME Temperature']));
  altitudeChart = new Chart(altitudeCtx, createChartConfig('Altitude', ['Altitude']));
  pressureChart = new Chart(pressureCtx, createChartConfig('Pressure', ['Pressure (hPa)']));
  humidityChart = new Chart(humidityCtx, createChartConfig('Humidity', ['Humidity (%)'])); // New humidity chart

  window.addEventListener('resize', () => {
    gyroChart.resize();
    accelChart.resize();
    tempChart.resize();
    altitudeChart.resize();
    pressureChart.resize();
    humidityChart.resize(); // Resize humidity chart on window resize
  });
}

function createChartConfig(title, labels) {
  return {
    type: 'line',
    data: {
      labels: [],
      datasets: labels.map((label, index) => ({
        label: `${title} ${label}`,
        data: [],
        borderColor: ['#61dafb', '#fb61da', '#dafb61', '#61fbda', '#da61fb', '#fbda61'][index],
        backgroundColor: [`rgba(97, 218, 251, 0.1)`, `rgba(251, 97, 218, 0.1)`, `rgba(218, 251, 97, 0.1)`, `rgba(97, 251, 218, 0.1)`, `rgba(218, 97, 251, 0.1)`, `rgba(251, 218, 97, 0.1)`][index],
        tension: 0.4
      }))
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        x: {
          type: 'linear',
          position: 'bottom',
          grid: { color: '#444' },
          ticks: { color: '#e0e0e0' }
        },
        y: {
          grid: { color: '#444' },
          ticks: { color: '#e0e0e0' }
        }
      },
      plugins: {
        legend: { labels: { color: '#e0e0e0' } }
      },
      animation: { duration: 0 }
    }
  };
}
function updateGyroChart(values) {
  updateChart(gyroChart, values);
}

function updateAccelChart(values) {
  updateChart(accelChart, values);
}

function updateTempChart(values) {
  updateChart(tempChart, values);
}
function updateAltitudeChart(values) {
  updateChart(altitudeChart, values);
}
function updatePressureChart(values) {
  updateChart(pressureChart, values);
}
function updateHumidityChart(values) {
  updateChart(humidityChart, values);
}
function updateChart(chart, values) {
  const time = Date.now();
  chart.data.labels.push(time);
  chart.data.datasets.forEach((dataset, index) => {
    dataset.data.push(values[index]);
  });

  if (chart.data.labels.length > 50) {
    chart.data.labels.shift();
    chart.data.datasets.forEach(dataset => dataset.data.shift());
  }
  chart.update('none');
}

function updateDataDisplay(values) {
  document.getElementById('gyro-display').textContent = 
    `Gyro X: ${values[0].toFixed(2)}, Gyro Y: ${values[1].toFixed(2)}, Gyro Z: ${values[2].toFixed(2)}`;
  document.getElementById('accel-display').textContent = 
    `Accel X: ${values[3].toFixed(2)}, Accel Y: ${values[4].toFixed(2)}, Accel Z: ${values[5].toFixed(2)}`;
  document.getElementById('temp-display').textContent = 
    `Temperature: ${values[6].toFixed(2)}, BME Temperature: ${values[7].toFixed(2)}`;
  document.getElementById('altitude-display').textContent = 
    `Altitude: ${values[8].toFixed(2)}`;
  document.getElementById('pressure-display').textContent = 
    `Pressure: ${values[9].toFixed(2)} hPa`;
  document.getElementById('humidity-display').textContent = 
    `Humidity: ${values[10].toFixed(2)}%`;
}
function showError(message) {
  const errorElement = document.getElementById('error-message');
  errorElement.textContent = message;
  errorElement.style.display = message ? 'block' : 'none';
}

function exportData() {
  const csvContent = "data:text/csv;charset=utf-8," 
    + "Time,GyroX,GyroY,GyroZ,AccelX,AccelY,AccelZ,Temperature,BMETemperature,Altitude,Pressure,Humidity\n"
    + dataLog.map(entry => {
      return `${entry.time},${entry.values.join(',')}`;
    }).join("\n");
  const encodedUri = encodeURI(csvContent);
  const link = document.createElement("a");
  link.setAttribute("href", encodedUri);
  link.setAttribute("download", "sensor_data.csv");
  document.body.appendChild(link);
  link.click();
  document.body.removeChild(link);
}

document.addEventListener('DOMContentLoaded', () => {
  initSerialPort();
  initCharts();
  document.getElementById('connect-button').addEventListener('click', toggleConnection);
  document.getElementById('export-button').addEventListener('click', exportData);
});

document.addEventListener('DOMContentLoaded', () => {
    const homeButton = document.getElementById('home-button');
    const chartLinks = document.querySelectorAll('.chart-link');
    const chartContainers = {
        gyro: document.getElementById('gyro-chart').parentElement,
        accel: document.getElementById('accel-chart').parentElement,
        temp: document.getElementById('temp-chart').parentElement,
        altitude: document.getElementById('altitude-chart').parentElement,
        pressure: document.getElementById('pressure-chart').parentElement,
        humidity: document.getElementById('humidity-chart').parentElement
    };
    const textContainers = {
        gyro: document.getElementById('gyro-display'),
        accel: document.getElementById('accel-display'),
        temp: document.getElementById('temp-display'),
        altitude: document.getElementById('altitude-display'),
        pressure: document.getElementById('pressure-display'),
        humidity: document.getElementById('humidity-display')
    };

    function resetView() {
        for (let chart in chartContainers) {
            chartContainers[chart].classList.remove('fullscreen');
            chartContainers[chart].style.display = 'block';
            textContainers[chart].style.display = 'block';
        }
    }

    homeButton.addEventListener('click', () => {
        resetView();
    });

    chartLinks.forEach(link => {
        link.addEventListener('click', (event) => {
            resetView();
            const selectedChart = event.target.getAttribute('data-chart');
            for (let chart in chartContainers) {
                if (chart !== selectedChart) {
                    chartContainers[chart].style.display = 'none';
                    textContainers[chart].style.display = 'none';
                }
            }
            chartContainers[selectedChart].classList.add('fullscreen');
        });
    });
});
