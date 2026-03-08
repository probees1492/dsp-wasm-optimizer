#!/usr/bin/env node

const https = require('https');

// GitHub API 토큰 없이 (public repo) 시도

const repoData = JSON.stringify({
    name: 'dsp-wasm-optimizer',
    description: 'C99 DSP 라이브러리를 WebAssembly로 컴파일하여 웹에서 실시간 벤치마킹하는 MVP 프로젝트',
    private: false,
    auto_init: true
});

const options = {
    hostname: 'api.github.com',
    path: '/user/repos',
    method: 'POST',
    headers: {
        'User-Agent': 'Node.js',
        'Authorization': 'token ghp_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx' // GitHub PAT 필요
    },
    body: repoData
};

console.log('❌ GitHub 저장소 생성 실패!');
console.log('');
console.log('사용자가 직접 GitHub 저장소를 생성해야 합니다.');
console.log('');
console.log('📋 수동 저장소 생성 방법:');
console.log('');
console.log('1. https://github.com/new 로 이동');
console.log('2. 저장소 이름: dsp-wasm-optimizer');
console.log('3. 설명: C99 DSP → WebAssembly 벤치마킹 MVP');
console.log('4. Public 체크');
console.log('5. "Create repository" 클릭');
console.log('');
console.log('생성 후 다음 명령어 실행:');
console.log('');
console.log('git remote add origin git@github.com:probees1492/dsp-wasm-optimizer.git');
console.log('git branch -M main');
console.log('git push -u origin main');
console.log('');
console.log('GitHub PAT (Personal Access Token)이 필요합니다.');
console.log('생성 방법: GitHub Settings → Developer settings → Personal access tokens');
console.log('권한: repo, workflow');
