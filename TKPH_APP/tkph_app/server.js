const express = require('express');
const mongoose = require('mongoose');
const bodyParser = require('body-parser');

const app = express();
const port = process.env.PORT || 3000;

// Connect to MongoDB
mongoose.connect('mongodb://localhost/tyre_management', { useNewUrlParser: true, useUnifiedTopology: true });

// Create a schema for your data
const dataSchema = new mongoose.Schema({
  timestamp: Date,
  tkph: Number,
  tyrePressures: {
    frontLeft: Number,
    frontRight: Number,
    middleLeft: Number,
    middleRight: Number,
    rearLeft: Number,
    rearRight: Number
  }
});

const Data = mongoose.model('Data', dataSchema);

app.use(bodyParser.json());

// Endpoint to receive data
app.post('/api/log-data', async (req, res) => {
  try {
    const newData = new Data(req.body);
    await newData.save();
    res.status(201).send('Data logged successfully');
  } catch (error) {
    res.status(500).send('Error logging data');
  }
});

app.listen(port, () => {
  console.log(`Server running on port ${port}`);
});