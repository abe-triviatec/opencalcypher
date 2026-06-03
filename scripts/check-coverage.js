#!/usr/bin/env node
'use strict';

const fs = require('fs');
const path = require('path');

const threshold = Number(process.env.PUBLIC_TOOLS_COVERAGE_THRESHOLD || '90');
const repoRoot = path.resolve(__dirname, '..');
const summaryPath = path.join(repoRoot, 'coverage/summary.json');

if (!fs.existsSync(summaryPath)) {
  console.error('missing coverage/summary.json; regenerate this repository from the private source of truth');
  process.exit(1);
}

const summary = JSON.parse(fs.readFileSync(summaryPath, 'utf8'));
const publicTools = Number(summary?.combined?.lines);
const communityRuntime = Number(summary?.communityRuntime?.lines);
const opencalcypher = Number(summary?.opencalcypher?.lines);

for (const [name, value] of [
  ['public tools', publicTools],
  ['community runtime', communityRuntime],
  ['opencalcypher', opencalcypher],
]) {
  if (!(value > threshold)) {
    console.error(`${name} coverage ${value}% is not strictly above ${threshold}%`);
    process.exit(1);
  }
}

console.log(`COMMUNITY_RUNTIME_COVERAGE=${communityRuntime.toFixed(2)}%`);
console.log(`OPENCALCYPHER_COVERAGE=${opencalcypher.toFixed(2)}%`);
console.log(`PUBLIC_TOOLS_COVERAGE=${publicTools.toFixed(2)}%`);
