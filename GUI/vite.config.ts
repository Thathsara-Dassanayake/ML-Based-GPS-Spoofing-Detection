import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'
import { spawn } from 'child_process'
import path from 'path'
import { fileURLToPath } from 'url'
import fs from 'fs/promises'

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const ROOT_DIR = path.resolve(__dirname, '..');

import { ChildProcess } from 'child_process'

let currentProcess: ChildProcess | null = null;
const SPOOF_BIN = `D:\\UoM\\Final Year Project\\FYP GitHub\\ML-Based-GPS-Spoofing-Detection\\ML_model_2\\test_data\\New_Galileo_3_Spoofer_Static_NoMP_TruePosition_500MB.bin`;
const LEGIT_BIN = `D:\\UoM\\Final Year Project\\FYP GitHub\\ML-Based-GPS-Spoofing-Detection\\ML_model_2\\test_data\\New_clearsky_signal_C-1_500MB.bin`;

export default defineConfig({
  plugins: [
    react(),
    {
      name: 'ml-backend',
      configureServer(server) {
        // Body parsing for JSON
        server.middlewares.use((req, res, next) => {
          if (req.method === 'POST') {
            let body = '';
            req.on('data', chunk => { body += chunk.toString(); });
            req.on('end', () => {
              if (body) {
                try { (req as any).body = JSON.parse(body); } catch(e){}
              }
              next();
            });
          } else {
            next();
          }
        });

        server.middlewares.use(async (req, res, next) => {
          if (req.url === '/api/start' && req.method === 'POST') {
            const type = (req as any).body?.type;
            if (currentProcess) {
                currentProcess.kill();
                currentProcess = null;
            }
            
            // clear latest json so we don't read old states
            try { await fs.unlink(path.join(ROOT_DIR, 'latest_status.json')); } catch(e){}
            
            const targetBin = type === 'spoof' ? SPOOF_BIN : LEGIT_BIN;
            const pythonPath = path.join(ROOT_DIR, 'ml_env', 'Scripts', 'python.exe');
            
            currentProcess = spawn(pythonPath, [
                'run_stream_inference.py', 
                '--source_bin', targetBin,
                '--model_dir', path.join(ROOT_DIR, 'ML_model_2', 'Model')
            ], {
                cwd: ROOT_DIR
            });
            
            res.setHeader('Content-Type', 'application/json');
            res.end(JSON.stringify({ message: 'Stream inference started', type }));
            return;
          }

          if (req.url === '/api/stop' && req.method === 'POST') {
            if (currentProcess) {
                currentProcess.kill();
                currentProcess = null;
            }
            res.setHeader('Content-Type', 'application/json');
            res.end(JSON.stringify({ message: 'Stream inference stopped' }));
            return;
          }

          if (req.url === '/api/status' && req.method === 'GET') {
            res.setHeader('Content-Type', 'application/json');
            try {
                const statusFile = path.join(ROOT_DIR, 'latest_status.json');
                const data = await fs.readFile(statusFile, 'utf-8');
                res.end(data);
            } catch (e) {
                res.statusCode = 503;
                res.end(JSON.stringify({ status: 'Inactive', message: 'No live status' }));
            }
            return;
          }

          next();
        });
      }
    }
  ]
})
