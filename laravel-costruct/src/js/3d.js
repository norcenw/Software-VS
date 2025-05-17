import * as THREE from 'three';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';
import { GLTFLoader } from 'three/addons/loaders/GLTFLoader.js';

function setupScene(containerId, modelPath) {
    const container = document.getElementById(containerId);

    // Forza il container a 250px x 250px
    container.style.width = '250px';
    container.style.height = '250px';

    const scene = new THREE.Scene();
    const camera = new THREE.PerspectiveCamera(
        75,
        1, // Forza il rapporto larghezza/altezza a 1 (quadrato)
        0.1,
        1000
    );
    camera.position.z = 5;

    const renderer = new THREE.WebGLRenderer({ antialias: true, alpha: true });
    renderer.setSize(250, 250); // Forza le dimensioni del renderer
    renderer.setClearColor(0x000000, 0); // Sfondo trasparente
    container.appendChild(renderer.domElement);

    const directionalLight = new THREE.DirectionalLight(0xffffff, 1);
    directionalLight.position.set(5, 10, 5);
    scene.add(directionalLight);

    const ambientLight = new THREE.AmbientLight(0x404040);
    scene.add(ambientLight);

    const controls = new OrbitControls(camera, renderer.domElement);
    controls.enableDamping = true;
    controls.dampingFactor = 0.1;

    // Disabilita lo spostamento del tasto destro
    controls.mouseButtons = {
        LEFT: THREE.MOUSE.ROTATE,
        MIDDLE: THREE.MOUSE.DOLLY,
        RIGHT: null, // Nessuna azione con il tasto destro
    };

    const loadingText = document.createElement('div');
    loadingText.style.position = 'absolute';
    loadingText.style.top = '10px';
    loadingText.style.left = '10px';
    loadingText.style.color = 'white';
    loadingText.style.fontSize = '14px';
    loadingText.innerText = `Loading model for ${containerId}...`;
    container.appendChild(loadingText);

    let initialPosition, initialRotation;

    const loader = new GLTFLoader();
    loader.load(
        modelPath,
        (gltf) => {
            const model = gltf.scene;
            const box = new THREE.Box3().setFromObject(model);
            const center = box.getCenter(new THREE.Vector3());

            model.scale.set(70, 70, 70);
            model.position.set(-center.x, -center.y, -center.z);
            model.rotation.y = -Math.PI / 4;

            // Salva la posizione e rotazione iniziale
            initialPosition = model.position.clone();
            initialRotation = model.rotation.clone();

            scene.add(model);
            container.removeChild(loadingText);
        },
        (xhr) => {
            console.log(
                `Caricamento (${containerId}): ${(xhr.loaded / xhr.total) * 100}% completato`
            );
        },
        (error) => {
            console.error(`Errore durante il caricamento del modello (${containerId}):`, error);
            loadingText.innerText = `Errore durante il caricamento del modello per ${containerId}.`;
        }
    );

    container.addEventListener('contextmenu', (event) => {
        event.preventDefault(); // Evita il menu contestuale del browser
        if (scene.children.length > 2) { // Controlla se il modello è stato caricato
            const model = scene.children[2];
            if (model && initialPosition && initialRotation) {
                // Reset alla posizione e rotazione iniziale
                model.position.copy(initialPosition);
                model.rotation.copy(initialRotation);
            }
        }
    });

    function animate() {
        requestAnimationFrame(animate);
        controls.update();
        renderer.render(scene, camera);
    }
    animate();

    window.addEventListener('resize', () => {
        const aspect = 1; 
        camera.aspect = aspect;
        camera.updateProjectionMatrix();
        renderer.setSize(250, 250); 
    });
}

setupScene('motor-x', './resources/3d/Stepper_Motor_NEMA_17.glb');
setupScene('motor-y', './resources/3d/Stepper_Motor_NEMA_17.glb');
setupScene('motor-z', './resources/3d/Stepper_Motor_NEMA_17.glb');

