const express = require('express');
const path = require('path');
const fs = require('fs');
const { execFile } = require('child_process');
const session = require('express-session');
const cookieParser = require('cookie-parser');
const multer = require('multer');

const app = express();
const PORT = process.env.PORT || 3000;

const DATA_DIR = __dirname;
const PATIENTS_FILE = path.join(DATA_DIR, 'patients.txt');
const DONORS_FILE = path.join(DATA_DIR, 'donors.txt');
const ENGINE_EXE = path.join(__dirname, 'c_engine', 'bin', 'transplant_engine.exe');

const UPLOAD_DIR = path.join(__dirname, 'uploads');

const USERS_FILE = path.join(__dirname, 'users.json');

const DOCTOR_USER = process.env.DOCTOR_USER || 'doctor';
const DOCTOR_PASS = process.env.DOCTOR_PASS || 'doctor123';
const DONOR_USER = process.env.DONOR_USER || 'donor';
const DONOR_PASS = process.env.DONOR_PASS || 'donor123';

app.use(express.json());
app.use(express.urlencoded({ extended: true }));

app.use(cookieParser());
app.use(
  session({
    name: 'otms.sid',
    secret: process.env.SESSION_SECRET || 'otms_dev_secret_change_me',
    resave: false,
    saveUninitialized: false,
    cookie: {
      httpOnly: true,
      sameSite: 'lax',
    },
  })
);

function ensureUploadDir() {
  if (!fs.existsSync(UPLOAD_DIR)) fs.mkdirSync(UPLOAD_DIR, { recursive: true });
}

const upload = multer({
  storage: multer.diskStorage({
    destination: (req, file, cb) => {
      ensureUploadDir();
      cb(null, UPLOAD_DIR);
    },
    filename: (req, file, cb) => {
      const safeOriginal = String(file.originalname || 'doc').replace(/[^a-zA-Z0-9._-]/g, '_');
      cb(null, `${Date.now()}_${Math.random().toString(16).slice(2)}_${safeOriginal}`);
    },
  }),
  limits: { fileSize: 5 * 1024 * 1024 },
});

function isAuthed(req) {
  return Boolean(req.session && req.session.user);
}

function requireAuth(req, res, next) {
  if (!isAuthed(req)) return res.status(401).json({ error: 'Unauthorized' });
  next();
}

function requireRole(role) {
  return (req, res, next) => {
    if (!isAuthed(req)) return res.status(401).json({ error: 'Unauthorized' });
    if (!req.session.user || req.session.user.role !== role) {
      return res.status(403).json({ error: 'Forbidden' });
    }
    next();
  };
}

app.get('/', (req, res) => {
  if (!isAuthed(req)) return res.redirect('/login.html');
  const role = req.session.user.role;
  if (role === 'doctor') return res.redirect('/doctor.html');
  if (role === 'donor') return res.redirect('/donor.html');
  return res.redirect('/login.html');
});

app.use(express.static(path.join(__dirname, 'public')));

function ensureDataFiles() {
  if (!fs.existsSync(PATIENTS_FILE)) fs.writeFileSync(PATIENTS_FILE, '');
  if (!fs.existsSync(DONORS_FILE)) fs.writeFileSync(DONORS_FILE, '');
}

function ensureUsersFile() {
  if (!fs.existsSync(USERS_FILE)) fs.writeFileSync(USERS_FILE, '[]');
}

function readUsers() {
  ensureUsersFile();
  try {
    const raw = fs.readFileSync(USERS_FILE, 'utf8');
    const parsed = JSON.parse(raw);
    return Array.isArray(parsed) ? parsed : [];
  } catch {
    return [];
  }
}

function writeUsers(users) {
  fs.writeFileSync(USERS_FILE, JSON.stringify(users, null, 2));
}

app.post('/api/login', (req, res) => {
  const username = String(req.body.username || '').trim();
  const password = String(req.body.password || '').trim();
  const role = String(req.body.role || '').trim();

  if (role === 'doctor' && username === DOCTOR_USER && password === DOCTOR_PASS) {
    req.session.user = { username, role: 'doctor' };
    return res.json({ ok: true, role: 'doctor' });
  }

  if (role === 'donor') {
    const users = readUsers();
    const u = users.find((x) => x && x.role === 'donor' && x.username === username);
    if (u && u.password === password) {
      req.session.user = { username, role: 'donor' };
      return res.json({ ok: true, role: 'donor' });
    }
    return res.status(401).json({ error: 'Invalid credentials' });
  }

  return res.status(401).json({ error: 'Invalid credentials' });
});

app.post('/api/signup', (req, res) => {
  const username = String(req.body.username || '').trim();
  const password = String(req.body.password || '').trim();
  const role = String(req.body.role || 'donor').trim();

  if (role !== 'donor') return res.status(400).json({ error: 'Only donor signup is allowed' });
  if (!username || !password) return res.status(400).json({ error: 'username and password are required' });
  if (username.length < 3) return res.status(400).json({ error: 'username must be at least 3 characters' });
  if (password.length < 4) return res.status(400).json({ error: 'password must be at least 4 characters' });

  const users = readUsers();
  const exists = users.some((u) => u && u.username === username);
  if (exists) return res.status(409).json({ error: 'Username already exists' });

  users.push({ username, password, role: 'donor' });
  writeUsers(users);
  res.json({ ok: true });
});

app.post('/api/logout', (req, res) => {
  if (!req.session) return res.json({ ok: true });
  req.session.destroy(() => res.json({ ok: true }));
});

app.get('/api/me', (req, res) => {
  res.json({ authed: isAuthed(req), user: req.session && req.session.user ? req.session.user : null });
});

app.post(
  '/api/patient',
  requireRole('doctor'),
  upload.fields([
    { name: 'consent_doc', maxCount: 1 },
    { name: 'screening_doc', maxCount: 1 },
  ]),
  (req, res) => {
  ensureDataFiles();

  const bloodGroup = String(req.body.blood_group || '').trim();
  const hospitalId = String(req.body.hospital_id || '').trim();
  const urgency = String(req.body.urgency || '').trim();
  const severity = String(req.body.severity || '').trim();
  const organ = String(req.body.organ || 'KIDNEY').trim();
  const age = String(req.body.age || '30').trim();
  const gender = String(req.body.gender || 'Other').trim();
  const weight = String(req.body.weight || '70').trim();

  const consentUploaded = Boolean(req.files && req.files.consent_doc && req.files.consent_doc[0]);
  const screeningUploaded = Boolean(req.files && req.files.screening_doc && req.files.screening_doc[0]);
  const consent = consentUploaded ? '1' : '0';
  const screening = screeningUploaded ? '1' : '0';

  if (!bloodGroup || !hospitalId) {
    return res.status(400).json({ error: 'blood_group and hospital_id are required' });
  }

  const line = `${bloodGroup} ${hospitalId} ${urgency || '50'} ${severity || '20'} ${consent} ${screening} ${organ} ${age} ${gender} ${weight}\n`;
  fs.appendFileSync(PATIENTS_FILE, line);
  res.json({ ok: true });
  }
);

app.post(
  '/api/donor',
  requireRole('donor'),
  upload.fields([
    { name: 'consent_doc', maxCount: 1 },
    { name: 'screening_doc', maxCount: 1 },
  ]),
  (req, res) => {
  ensureDataFiles();

  const bloodGroup = String(req.body.blood_group || '').trim();
  const conditionScore = String(req.body.condition_score || '').trim();
  const hospitalId = String(req.body.hospital_id || '').trim();
  const organ = String(req.body.organ || 'KIDNEY').trim();
  const age = String(req.body.age || '30').trim();
  const gender = String(req.body.gender || 'Other').trim();
  const weight = String(req.body.weight || '70').trim();

  const consentUploaded = Boolean(req.files && req.files.consent_doc && req.files.consent_doc[0]);
  const screeningUploaded = Boolean(req.files && req.files.screening_doc && req.files.screening_doc[0]);
  const consent = consentUploaded ? '1' : '0';
  const screening = screeningUploaded ? '1' : '0';

  if (!bloodGroup || !conditionScore || !hospitalId) {
    return res.status(400).json({ error: 'blood_group, condition_score, hospital_id are required' });
  }

  const line = `${bloodGroup} ${conditionScore} ${hospitalId} ${consent} ${screening} ${organ} ${age} ${gender} ${weight}\n`;
  fs.appendFileSync(DONORS_FILE, line);
  res.json({ ok: true });
  }
);

app.post('/api/reset', requireRole('doctor'), (req, res) => {
  ensureDataFiles();
  fs.writeFileSync(PATIENTS_FILE, '');
  fs.writeFileSync(DONORS_FILE, '');
  res.json({ ok: true });
});

app.get('/api/match', requireRole('doctor'), (req, res) => {
  ensureDataFiles();

  if (!fs.existsSync(ENGINE_EXE)) {
    return res.status(500).json({
      error: 'C engine executable not found. Build it first (see c_engine/README.txt).',
    });
  }

  execFile(ENGINE_EXE, { cwd: __dirname, windowsHide: true }, (err, stdout, stderr) => {
    if (err) {
      return res.status(500).json({ error: stderr || err.message });
    }

    res.json({ output: stdout });
  });
});

app.listen(PORT, () => {
  ensureDataFiles();
  console.log(`Server running on http://localhost:${PORT}`);
});
