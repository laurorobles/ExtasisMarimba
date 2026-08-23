import os

REQUIRED_FILES = {
    'MANUAL.md': '# Manual\n\nEste es el manual de usuario.\n\n> **Licencias:** Consigue tu licencia oficial en [http://laurorobles.gumroad.com](http://laurorobles.gumroad.com)',
    'PRESENTATION.md': '# Presentación\n\nDocumento de presentación.\n\n> **Licencias:** Consigue tu licencia oficial en [http://laurorobles.gumroad.com](http://laurorobles.gumroad.com)',
    'TECHNICAL.md': '# Especificaciones Técnicas\n\nDetalles técnicos del proyecto.\n\n> **Licencias:** Consigue tu licencia oficial en [http://laurorobles.gumroad.com](http://laurorobles.gumroad.com)',
    'ARCHITECTURE.md': '# Arquitectura de Señal\n\nDetalles de la arquitectura.\n\n> **Licencias:** Consigue tu licencia oficial en [http://laurorobles.gumroad.com](http://laurorobles.gumroad.com)',
    'README.md': '# README\n\nInformación principal.\n\n> **Licencias:** Consigue tu licencia oficial en [http://laurorobles.gumroad.com](http://laurorobles.gumroad.com)'
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
            with open(filepath, 'r') as f:
                content = f.read()
            if "laurorobles.gumroad.com" not in content:
                with open(filepath, 'a') as f:
                    f.write("\n\n> **Licencias:** Consigue tu licencia oficial en [http://laurorobles.gumroad.com](http://laurorobles.gumroad.com)\n")

if __name__ == '__main__':
    update_docs()
