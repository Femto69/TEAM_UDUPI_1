const { SerialPort } = require('serialport');
const { parse } = require('csv-parse');

let map;
let chart;
const aqiData = [];
const coData = [];
const smokeData = [];

document.addEventListener('DOMContentLoaded', () => {
    console.log('DOM fully loaded and parsed');
    initMap();
    initCharts();
    initVideoFeed();
    populateAQITable();  // This will now use your predefined values
});
function initMap() {
    console.log('Initializing map');
    const delhi = [28.7497093, 77.1148453];
    
    try {
        map = L.map('map').setView(delhi, 17);

        console.log('Map object created');

        L.tileLayer('https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}', {
            attribution: 'Tiles &copy; Esri &mdash; Source: Esri, i-cubed, USDA, USGS, AEX, GeoEye, Getmapping, Aerogrid, IGN, IGP, UPR-EGP, and the GIS User Community'
        }).addTo(map);

        console.log('Esri World Imagery layer added to map');

        addMarker(delhi[0], delhi[1], 'Delhi, India', 0);

        console.log('Initial marker added to map');
    } catch (error) {
        console.error('Error initializing map:', error);
    }
}

function initCharts() {
    const aqiCtx = document.getElementById('aqi-chart').getContext('2d');
    const coCtx = document.getElementById('co-chart').getContext('2d');
    const smokeCtx = document.getElementById('smoke-chart').getContext('2d');

    aqiChart = createChart(aqiCtx, 'AQI');
    coChart = createChart(coCtx, 'CO');
    smokeChart = createChart(smokeCtx, 'Smoke');
}

function createChart(ctx, label) {
    return new Chart(ctx, {
        type: 'bar',
        data: {
            labels: [],
            datasets: [{
                label: label,
                data: [],
                backgroundColor: 'rgba(75, 192, 192, 0.6)',
                borderColor: 'rgba(75, 192, 192, 1)',
                borderWidth: 1
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            scales: {
                y: {
                    beginAtZero: true,
                    grid: {
                        color: 'rgba(255, 255, 255, 0.1)'
                    },
                    ticks: {
                        color: '#ffffff'
                    }
                },
                x: {
                    grid: {
                        color: 'rgba(255, 255, 255, 0.1)'
                    },
                    ticks: {
                        color: '#ffffff'
                    }
                }
            },
            plugins: {
                legend: {
                    labels: {
                        color: '#ffffff'
                    }
                }
            }
        }
    });
}

function initVideoFeed() {
    const videoElement = document.getElementById('videoFeed');

    if (navigator.mediaDevices && navigator.mediaDevices.getUserMedia) {
        navigator.mediaDevices.getUserMedia({ video: true })
            .then((stream) => {
                videoElement.srcObject = stream;
            })
            .catch((error) => {
                console.error("Error accessing the video feed:", error);
            });
    } else {
        console.error("getUserMedia is not supported in this browser");
    }
}

function getColor(aqi) {
    if (aqi > 400) return 'red';
    if (aqi > 200) return 'orange';
    return 'green';  // Default color for AQI <= 200
}

function getRadius(aqi) {
    if (aqi > 400) return 2000;  // Larger radius for higher AQI
    if (aqi > 200) return 1500;
    return 200;  // Default radius for AQI <= 200
}

function addMarker(lat, lon, label, aqi) {
    console.log(`Adding marker at ${lat}, ${lon} with label: ${label} and AQI: ${aqi}`);
    try {
        const marker = L.marker([lat, lon]).addTo(map)
            .bindPopup(`${label}<br>AQI: ${aqi}`)
            .openPopup();

        const circle = L.circle([lat, lon], {
            color: getColor(aqi),
            fillColor: getColor(aqi),
            fillOpacity: 0.2,
            radius: getRadius(aqi)
        }).addTo(map);

        marker.circle = circle;

        return marker;
    } catch (error) {
        console.error('Error adding marker:', error);
    }
}

function updateMarker(marker, newAqi) {
    const lat = marker.getLatLng().lat;
    const lon = marker.getLatLng().lng;
    const label = marker.getPopup().getContent().split('<br>')[0];

    marker.setPopupContent(`${label}<br>AQI: ${newAqi}`);

    if (marker.circle) {
        marker.circle.setStyle({
            color: getColor(newAqi),
            fillColor: getColor(newAqi),
            radius: getRadius(newAqi)
        });
    }
}

function updateCharts(newAqi, newCo, newSmoke) {
    const now = new Date();
    const timeString = now.toLocaleTimeString();
    
    updateSingleChart(aqiData, aqiChart, timeString, newAqi);
    updateSingleChart(coData, coChart, timeString, newCo);
    updateSingleChart(smokeData, smokeChart, timeString, newSmoke);
}

function updateSingleChart(dataArray, chart, timeString, newValue) {
    dataArray.push({ time: timeString, value: newValue });
    if (dataArray.length > 300) dataArray.shift();

    chart.data.labels = dataArray.map(d => d.time);
    chart.data.datasets[0].data = dataArray.map(d => d.value);
    chart.data.datasets[0].backgroundColor = dataArray.map(d => getColor(d.value));
    chart.data.datasets[0].borderColor = dataArray.map(d => getColor(d.value));
    chart.update();
}

function processSerialData(data) {
    parse(data, (err, records) => {
        if (err) {
            console.error('Error parsing CSV:', err);
            return;
        }
        records.forEach(record => {
            let smoke = parseInt(record[0]);
            let co = parseInt(record[1]);
            let aqi = parseInt(record[2]);
            
            let existingMarker = findMarkerByLatLng(28.7497093, 77.1148453);
            if (existingMarker) {
                updateMarker(existingMarker, aqi);
            } else {
                addMarker(28.7497093, 77.1148453, 'Delhi, India', aqi);
            }
            updateCharts(aqi, co, smoke);
        });
    });
}

function findMarkerByLatLng(lat, lng) {
    let found = null;
    map.eachLayer((layer) => {
        if (layer instanceof L.Marker) {
            const latLng = layer.getLatLng();
            if (latLng.lat === lat && latLng.lng === lng) {
                found = layer;
            }
        }
    });
    return found;
}


function populateAQITable() {
    const tableBody = document.getElementById('aqi-table-body');
    const today = new Date();
    
    // Predefined data for the last three days
    const aqiData = [
        { date: new Date(today.getTime() - 2 * 24 * 60 * 60 * 1000), predicted: 142, real: 164 },
        { date: new Date(today.getTime() - 1 * 24 * 60 * 60 * 1000), predicted: 188, real: 153 },
        { date: new Date(today.getTime()), predicted: 148, real: 164 }
    ];
    
    aqiData.forEach(data => {
        const row = tableBody.insertRow();
        
        const dateCell = row.insertCell(0);
        dateCell.textContent = data.date.toLocaleDateString();
        
        const predictedCell = row.insertCell(1);
        predictedCell.textContent = data.predicted;
        
        const realCell = row.insertCell(2);
        realCell.textContent = data.real;
    });
}
// Set up serial port listeners
['COM3'].forEach(portName => {
    console.log(`Attempting to open serial port: ${portName}`);
    const port = new SerialPort({ path: portName, baudRate: 115200 });
    port.on('open', () => console.log(`Port ${portName} opened`));
    port.on('error', (err) => console.error(`Error on ${portName}:`, err));
    port.on('data', processSerialData);
});