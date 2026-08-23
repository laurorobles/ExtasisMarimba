import os

REQUIRED_FILES = {
    'MANUAL.md': '# Manual\n\nEste es el manual de usuario.',
    'PRESENTATION.md': '# Presentación\n\nDocumento de presentación.',
    'TECHNICAL.md': '# Especificaciones Técnicas\n\nDetalles técnicos del proyecto.',
    'ARCHITECTURE.md': '# Arquitectura de Señal\n\nDetalles de la arquitectura.',
    'README.md': '# README\n\nInformación principal.'
}

def update_docs():
    project_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    for filename, default_content in REQUIRED_FILES.items():
        filepath = os.path.join(project_dir, filename)
        if not os.path.exists(filepath):
            print(f"Creando {filepath}...")
            with open(filepath, 'w') as f:
                f.write(default_content)
        else:
            print(f"{filename} ya existe en el proyecto, verificando...")

if __name__ == '__main__':
    update_docs()
