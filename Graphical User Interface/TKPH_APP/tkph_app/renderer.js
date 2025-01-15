const { ipcRenderer } = require('electron');
const axios = require('axios');

let tkphChart, pressureChart;
const MAX_DATA_POINTS = 20;

// Initialize charts
function initCharts() {
    tkphChart = new Chart(document.getElementById('tkphChart'), {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'TKPH',
                data: [],
                borderColor: 'rgb(75, 192, 192)',
                tension: 0.1
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            scales: {
                x: {
                    ticks: {
                        maxTicksLimit: 10
                    }
                }
            }
        }
    });

    pressureChart = new Chart(document.getElementById('pressureChart'), {
        type: 'line',
        data: {
            labels: [],
            datasets: [
                { label: 'Front Left', data: [], borderColor: 'red', tension: 0.1 },
                { label: 'Front Right', data: [], borderColor: 'blue', tension: 0.1 },
                { label: 'Middle Left', data: [], borderColor: 'green', tension: 0.1 },
                { label: 'Middle Right', data: [], borderColor: 'purple', tension: 0.1 },
                { label: 'Rear Left', data: [], borderColor: 'orange', tension: 0.1 },
                { label: 'Rear Right', data: [], borderColor: 'brown', tension: 0.1 }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            scales: {
                x: {
                    ticks: {
                        maxTicksLimit: 10
                    }
                }
            }
        }
    });
}

function updateTKPHDisplay(data) {
    const tkph = (data.load * data.speed).toFixed(2);
    console.log('Calculated TKPH:', tkph); // Add this line for debugging

    document.getElementById('current-tkph').textContent = tkph;
    document.getElementById('mean-load-value').textContent = data.load;
    document.getElementById('avg-speed-value').textContent = data.speed;

    // Update TKPH chart
    const timestamp = new Date().toLocaleTimeString();
    tkphChart.data.labels.push(timestamp);
    tkphChart.data.datasets[0].data.push(parseFloat(tkph));

    console.log('Chart data:', tkphChart.data); // Add this line for debugging

    if (tkphChart.data.labels.length > MAX_DATA_POINTS) {
        tkphChart.data.labels.shift();
        tkphChart.data.datasets[0].data.shift();
    }

    tkphChart.update();
}

function updateTruckVisualization(data) {
    const svgDoc = document.getElementById('truck-svg').contentDocument;

    if (svgDoc) {
        const updateTyrePressure = (id, value) => {
            const element = svgDoc.getElementById(id);
            if (element) {
                element.textContent = value;
                // Color coding based on pressure
                const color = value < 80 ? 'red' : value < 90 ? 'yellow' : 'green';
                element.setAttribute('fill', color);
            }
        };

        updateTyrePressure('front-left-pressure', data.frontLeft);
        updateTyrePressure('front-right-pressure', data.frontRight);
        updateTyrePressure('middle-left-pressure', data.middleLeft);
        updateTyrePressure('middle-right-pressure', data.middleRight);
        updateTyrePressure('rear-left-pressure', data.rearLeft);
        updateTyrePressure('rear-right-pressure', data.rearRight);
    }
}

function updatePressureChart(data) {
    const timestamp = new Date().toLocaleTimeString();
    pressureChart.data.labels.push(timestamp);
    pressureChart.data.datasets[0].data.push(data.frontLeft);
    pressureChart.data.datasets[1].data.push(data.frontRight);
    pressureChart.data.datasets[2].data.push(data.middleLeft);
    pressureChart.data.datasets[3].data.push(data.middleRight);
    pressureChart.data.datasets[4].data.push(data.rearLeft);
    pressureChart.data.datasets[5].data.push(data.rearRight);

    if (pressureChart.data.labels.length > MAX_DATA_POINTS) {
        pressureChart.data.labels.shift();
        pressureChart.data.datasets.forEach(dataset => dataset.data.shift());
    }

    pressureChart.update();
}

async function logDataToServer(data) {
    try {
        await axios.post('http://localhost:3000/api/log-data', {
            timestamp: new Date(),
            tkph: (data.load * data.speed).toFixed(2),
            tyrePressures: {
                frontLeft: data.frontLeft,
                frontRight: data.frontRight,
                middleLeft: data.middleLeft,
                middleRight: data.middleRight,
                rearLeft: data.rearLeft,
                rearRight: data.rearRight
            }
        });
        console.log('Data logged successfully');
    } catch (error) {
        console.error('Error logging data:', error);
    }
}

// Update tyre pressure display and charts
ipcRenderer.on('tyre-data', (event, data) => {
    console.log('Received tyre data:', data); // Add this line for debugging

    document.querySelector('#front-left span').textContent = data.frontLeft;
    document.querySelector('#front-right span').textContent = data.frontRight;
    document.querySelector('#middle-left span').textContent = data.middleLeft;
    document.querySelector('#middle-right span').textContent = data.middleRight;
    document.querySelector('#rear-left span').textContent = data.rearLeft;
    document.querySelector('#rear-right span').textContent = data.rearRight;

    updateTruckVisualization(data);
    updateTKPHDisplay(data);
    updatePressureChart(data);
    logDataToServer(data);
});

// Maintenance log functionality
document.getElementById('add-maintenance').addEventListener('click', () => {
    const entry = document.getElementById('maintenance-entry').value;
    if (entry) {
        const listItem = document.createElement('li');
        listItem.textContent = `${new Date().toLocaleString()}: ${entry}`;
        document.getElementById('maintenance-list').appendChild(listItem);
        document.getElementById('maintenance-entry').value = '';
    }
});

// Initialize charts when the page loads
document.addEventListener('DOMContentLoaded', initCharts);