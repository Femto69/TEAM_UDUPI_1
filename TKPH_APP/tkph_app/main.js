const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const fs = require('fs');
const csv = require('csv-parser');

let mainWindow;
let csvData = [];
let currentRow = 0;

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1200,
    height: 800,
    webPreferences: {
      nodeIntegration: true,
      contextIsolation: false,
    },
  });

  mainWindow.loadFile('index.html');
  mainWindow.webContents.openDevTools(); // Open DevTools for debugging
}

app.whenReady().then(() => {
  createWindow();
  loadCSVData();
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow();
  }
});

function loadCSVData() {
  const csvFilePath = path.join(__dirname, 'tkph_data.csv');
  console.log('Attempting to read CSV file from:', csvFilePath);

  fs.createReadStream(csvFilePath)
    .pipe(csv())
    .on('data', (row) => {
      csvData.push(row);
      if (csvData.length === 1) {
        console.log('CSV Headers:', Object.keys(row));
      }
    })
    .on('end', () => {
      console.log('CSV file successfully processed. Row count:', csvData.length);
      if (csvData.length > 0) {
        console.log('First row of data:', JSON.stringify(csvData[0], null, 2));
      }
      startDataSimulation();
    })
    .on('error', (error) => {
      console.error('Error reading CSV file:', error);
    });
}

function startDataSimulation() {
  console.log('Starting data simulation');
  setInterval(() => {
    if (currentRow >= csvData.length) {
      currentRow = 0;
    }
    
    const row = csvData[currentRow];
    console.log('Sending data for row:', currentRow);
    console.log('Raw row data:', JSON.stringify(row, null, 2));
    
    const data = {
      frontLeft: row.Tyre1_Pressure,
      frontRight: row.Tyre2_Pressure,
      middleLeft: row.Tyre3_Pressure,
      middleRight: row.Tyre4_Pressure,
      rearLeft: row.Tyre5_Pressure,
      rearRight: row.Tyre6_Pressure,
      load: row.Load_Tonnes,
      speed: row.Speed_KmH
    };
    
    console.log('Processed data being sent:', JSON.stringify(data, null, 2));
    
    mainWindow.webContents.send('tyre-data', data);
    
    currentRow++;
  }, 1000); // Send data every second
}