import express from 'express';
import cors from 'cors';
import { spawn } from 'child_process';
import path from 'path';
import { fileURLToPath } from 'url';
import fs from 'fs/promises';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const app = express();
app.use(cors());
app.use(express.json());

const PORT = process.env.PORT || 3001;
const ROOT_DIR = path.resolve(__dirname, '..');

let currentProcess = null;

const SPOOF_BIN = `D:\\UoM\\Final Year Project\\FYP GitHub\\ML-Based-GPS-Spoofing-Detection\\ML_model_2\\test_data\\New_Galileo_3_Spoofer_Static_NoMP_TruePosition_500MB.bin`;
const LEGIT_BIN = `D:\\UoM\\Final Year Project\\FYP GitHub\\ML-Based-GPS-Spoofing-Detection\\ML_model_2\\test_data\\New_clearsky_signal_C-1_500MB.bin`;

app.post('/api/start', (req, res) => {
    const { type } = req.body;
    
    if (currentProcess) {
        currentProcess.kill();
        currentProcess = null;
    }
    
    // We clear the latest json so we don't read old states
    fs.unlink(path.join(ROOT_DIR, 'latest_status.json')).catch(() => {});
    
    const targetBin = type === 'spoof' ? SPOOF_BIN : LEGIT_BIN;
    
    const pythonPath = path.join(ROOT_DIR, 'ml_env', 'Scripts', 'python.exe');
    
    // Spawn python process
    currentProcess = spawn(pythonPath, [
        'run_stream_inference.py', 
        '--source_bin', targetBin,
        '--model_dir', path.join(ROOT_DIR, 'ML_model_2', 'Model')
    ], {
        cwd: ROOT_DIR
    });
    
    currentProcess.stdout.on('data', (data) => {
        console.log(`[Python]: ${data}`);
    });
    
    currentProcess.stderr.on('data', (data) => {
        console.error(`[Python Err]: ${data}`);
    });
    
    res.json({ message: 'Stream inference started', type });
});

app.post('/api/stop', (req, res) => {
    if (currentProcess) {
        currentProcess.kill();
        currentProcess = null;
    }
    res.json({ message: 'Stream inference stopped' });
});

app.get('/api/status', async (req, res) => {
    try {
        const statusFile = path.join(ROOT_DIR, 'latest_status.json');
        const data = await fs.readFile(statusFile, 'utf-8');
        res.json(JSON.parse(data));
    } catch (e) {
        res.status(503).json({ status: 'Inactive', message: 'No live status' });
    }
});

app.listen(PORT, () => {
    console.log(`Backend server listening on port ${PORT}`);
});
