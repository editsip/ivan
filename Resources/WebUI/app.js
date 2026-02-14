// ============================================================================
// Ivan Valve Amplifier — UI Controller
// Handles knob interaction, view switching, parameter sync, and tube viz
// ============================================================================

(function() {
    'use strict';

    // Parameter state
    const params = {};

    // ---- Knob interaction ----

    function initKnobs() {
        document.querySelectorAll('.knob').forEach(knob => {
            const param = knob.dataset.param;
            const min = parseFloat(knob.dataset.min);
            const max = parseFloat(knob.dataset.max);
            const def = parseFloat(knob.dataset.default);

            params[param] = def;
            updateKnobVisual(knob, def, min, max);

            let dragging = false;
            let startY = 0;
            let startVal = 0;

            knob.addEventListener('mousedown', e => {
                dragging = true;
                startY = e.clientY;
                startVal = params[param];
                e.preventDefault();
                document.body.style.cursor = 'ns-resize';
            });

            // Double-click to reset
            knob.addEventListener('dblclick', e => {
                params[param] = def;
                updateKnobVisual(knob, def, min, max);
                sendParam(param, def);
                e.preventDefault();
            });

            document.addEventListener('mousemove', e => {
                if (!dragging) return;
                const dy = startY - e.clientY;
                const range = max - min;
                const sensitivity = e.shiftKey ? 600 : 200;
                let newVal = startVal + (dy / sensitivity) * range;
                newVal = Math.max(min, Math.min(max, newVal));
                params[param] = newVal;
                updateKnobVisual(knob, newVal, min, max);
                sendParam(param, newVal);
            });

            document.addEventListener('mouseup', () => {
                if (dragging) {
                    dragging = false;
                    document.body.style.cursor = '';
                }
            });
        });
    }

    function updateKnobVisual(knob, value, min, max) {
        const norm = (value - min) / (max - min);
        const isSmall = knob.closest('.knob-group')?.classList.contains('small');

        // Update arc (270 degree sweep)
        const circumference = isSmall ? 150.8 : 213.6;
        const arcLength = norm * circumference * 0.75; // 270 degrees
        const valueCircle = knob.querySelector('.knob-value');
        if (valueCircle) {
            valueCircle.style.strokeDashoffset = circumference - arcLength;
        }

        // Update dot position
        const dot = knob.querySelector('.knob-dot');
        if (dot) {
            const angle = -135 + norm * 270; // -135 to +135 degrees
            const r = isSmall ? 24 : 34;
            const cx = isSmall ? 30 : 40;
            const cy = isSmall ? 30 : 40;
            const rad = angle * Math.PI / 180;
            const dx = cx + r * Math.sin(rad);
            const dy = cy - r * Math.cos(rad);
            dot.setAttribute('cx', dx);
            dot.setAttribute('cy', dy);
        }

        // Update value text
        const group = knob.closest('.knob-group');
        const valueText = group?.querySelector('.knob-value-text');
        if (valueText) {
            valueText.textContent = formatValue(knob.dataset.param, value);
        }
    }

    function formatValue(param, value) {
        switch (param) {
            case 'inputGain':
            case 'outputLevel':
                return value.toFixed(1) + ' dB';
            case 'compThreshold':
            case 'gateThreshold':
                return value.toFixed(0) + 'dB';
            case 'compRatio':
                return value.toFixed(1) + ':1';
            case 'micDistance':
                return value.toFixed(0) + 'cm';
            case 'micAngle':
                return value.toFixed(0) + '\u00B0';
            case 'variac':
                return (value * 100).toFixed(0) + '%';
            case 'classAB':
                return value < 0.3 ? 'AB' : value > 0.7 ? 'A' : 'A/AB';
            default:
                return (value * 100).toFixed(0) + '%';
        }
    }

    // ---- Channel switching ----

    function initChannelTabs() {
        document.querySelectorAll('.channel-tab').forEach(tab => {
            tab.addEventListener('click', () => {
                document.querySelectorAll('.channel-tab').forEach(t => t.classList.remove('active'));
                tab.classList.add('active');
                sendParam('channel', parseInt(tab.dataset.channel));
            });
        });
    }

    // ---- View switching ----

    function initViewTabs() {
        document.querySelectorAll('.view-tab').forEach(tab => {
            tab.addEventListener('click', () => {
                document.querySelectorAll('.view-tab').forEach(t => t.classList.remove('active'));
                tab.classList.add('active');
                document.querySelectorAll('.view-content').forEach(v => v.classList.remove('active'));
                const viewId = 'view-' + tab.dataset.view;
                document.getElementById(viewId)?.classList.add('active');
            });
        });
    }

    // ---- Selector buttons ----

    function initSelectors() {
        document.querySelectorAll('.selector-btn').forEach(btn => {
            btn.addEventListener('click', () => {
                const param = btn.dataset.param;
                // Deselect siblings with same param
                btn.parentElement.querySelectorAll(`.selector-btn[data-param="${param}"]`).forEach(s => {
                    s.classList.remove('active');
                });
                btn.classList.add('active');
                sendParam(param, parseInt(btn.dataset.value));
            });
        });
    }

    // ---- Dropdown selects ----

    function initSelects() {
        document.querySelectorAll('.tube-select select').forEach(sel => {
            sel.addEventListener('change', () => {
                sendParam(sel.dataset.param, parseInt(sel.value));
                updateTubeVizLabel(sel);
            });
        });
    }

    function updateTubeVizLabel(sel) {
        if (sel.dataset.param === 'preampTube1') {
            const label = document.getElementById('tubeLabel');
            if (label) label.textContent = sel.options[sel.selectedIndex].text;
        }
    }

    // ---- Parameter communication ----
    // In the JUCE WebBrowser, we communicate via URL changes or
    // evaluateJavascript. For now, we expose a global handler.

    function sendParam(id, value) {
        // JUCE will intercept navigation to param:// URLs
        try {
            window.location.href = 'param://' + id + '/' + value;
        } catch(e) {
            // Fallback: store locally
        }
    }

    // Called by JUCE to update a parameter in the UI
    window.onParamChange = function(paramId, value) {
        params[paramId] = value;
        // Update corresponding knob
        const knob = document.querySelector(`.knob[data-param="${paramId}"]`);
        if (knob) {
            const min = parseFloat(knob.dataset.min);
            const max = parseFloat(knob.dataset.max);
            updateKnobVisual(knob, value, min, max);
        }
    };

    // Called by JUCE for metering updates
    window.updateMeters = function(gainReduction, gateOpen) {
        const gateIndicator = document.getElementById('gateIndicator');
        if (gateIndicator) {
            gateIndicator.classList.toggle('open', gateOpen);
        }
        const meterFill = document.querySelector('#grMeter .meter-fill');
        if (meterFill) {
            const gr = Math.min(100, Math.abs(gainReduction) * 3);
            meterFill.style.width = gr + '%';
        }
    };

    // ---- Tube wireframe visualization ----

    function initTubeViz() {
        const canvas = document.getElementById('tubeViz');
        if (!canvas) return;
        const ctx = canvas.getContext('2d');
        const w = canvas.width;
        const h = canvas.height;

        let frame = 0;

        function drawTube() {
            ctx.clearRect(0, 0, w, h);

            const cx = w / 2;
            const cy = h / 2;

            ctx.strokeStyle = 'rgba(196, 136, 58, 0.25)';
            ctx.lineWidth = 0.8;

            // Draw tube bottle (elliptical wireframe)
            const segments = 24;
            const rings = 16;
            const radiusX = 60;
            const radiusZ = 60;
            const height = 200;

            for (let r = 0; r < rings; r++) {
                const t = r / (rings - 1);
                const y = -height / 2 + t * height;

                // Bottle shape: bulge in the middle, narrow at top/bottom
                const bulge = Math.sin(t * Math.PI);
                const rx = radiusX * (0.3 + 0.7 * bulge);
                const rz = radiusZ * (0.3 + 0.7 * bulge);

                ctx.beginPath();
                for (let s = 0; s <= segments; s++) {
                    const angle = (s / segments) * Math.PI * 2 + frame * 0.003;
                    const px = cx + rx * Math.cos(angle);
                    const depth = rz * Math.sin(angle);
                    const py = cy + y * 0.8 + depth * 0.15;

                    // Perspective scaling
                    const scale = 1 + depth * 0.002;
                    const sx = cx + (px - cx) * scale;

                    if (s === 0) ctx.moveTo(sx, py);
                    else ctx.lineTo(sx, py);
                }
                ctx.stroke();
            }

            // Vertical lines
            for (let s = 0; s < segments; s++) {
                const angle = (s / segments) * Math.PI * 2 + frame * 0.003;
                ctx.beginPath();
                for (let r = 0; r < rings; r++) {
                    const t = r / (rings - 1);
                    const y = -height / 2 + t * height;
                    const bulge = Math.sin(t * Math.PI);
                    const rx = radiusX * (0.3 + 0.7 * bulge);
                    const rz = radiusZ * (0.3 + 0.7 * bulge);

                    const px = cx + rx * Math.cos(angle);
                    const depth = rz * Math.sin(angle);
                    const py = cy + y * 0.8 + depth * 0.15;
                    const scale = 1 + depth * 0.002;
                    const sx = cx + (px - cx) * scale;

                    if (r === 0) ctx.moveTo(sx, py);
                    else ctx.lineTo(sx, py);
                }
                ctx.stroke();
            }

            // Internal glow — simulates heater
            const glowIntensity = 0.15 + 0.05 * Math.sin(frame * 0.02);
            const gradient = ctx.createRadialGradient(cx, cy + 20, 5, cx, cy + 20, 50);
            gradient.addColorStop(0, `rgba(196, 136, 58, ${glowIntensity})`);
            gradient.addColorStop(1, 'rgba(196, 136, 58, 0)');
            ctx.fillStyle = gradient;
            ctx.fillRect(0, 0, w, h);

            // Plate structure inside (simplified)
            ctx.strokeStyle = 'rgba(196, 136, 58, 0.12)';
            ctx.lineWidth = 1;
            ctx.beginPath();
            ctx.rect(cx - 20, cy - 40, 40, 80);
            ctx.stroke();

            // Grid wires
            ctx.strokeStyle = 'rgba(196, 136, 58, 0.08)';
            ctx.lineWidth = 0.5;
            for (let i = -3; i <= 3; i++) {
                ctx.beginPath();
                ctx.moveTo(cx - 25, cy + i * 10);
                ctx.lineTo(cx + 25, cy + i * 10);
                ctx.stroke();
            }

            frame++;
            requestAnimationFrame(drawTube);
        }

        drawTube();
    }

    // ---- Init ----

    document.addEventListener('DOMContentLoaded', () => {
        initKnobs();
        initChannelTabs();
        initViewTabs();
        initSelectors();
        initSelects();
        initTubeViz();
    });

})();
