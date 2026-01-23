import json
import os
import logging

log = logging.getLogger("mkdocs.plugins")

# Global list for our flattened entries
flat_index = []

def on_page_content(html, page, config, files):
    """
    Called for every page. We extract the top-level page info 
    and then recursively flatten the TOC items.
    """
    # 1. Add the main page itself as an entry (H1)
    flat_index.append({
        "title": page.title,
        "url": page.abs_url,
        "level": 1,
        "parent_chapter": page.title
    })
    
    # 2. Add all sub-sections (H2, H3, etc.)
    if page.toc:
        for item in page.toc:
            _flatten_toc(item, page.abs_url, page.title)
            
    return html

def _flatten_toc(item, page_url, parent_title):
    """
    Recursively pulls nested TOC items into the top-level flat_index.
    """
    # Skip the H1 if it's identical to the page title (to avoid duplicates)
    if item.level > 1:
        flat_index.append({
            "title": item.title,
            "url": f"{page_url}{item.url}",
            "level": item.level,
            "parent_chapter": parent_title
        })
    
    # Process children (sub-headers)
    for child in getattr(item, 'children', []):
        _flatten_toc(child, page_url, parent_title)

def on_post_build(config):
    """Writes the flat array to a JSON file."""
    if not flat_index:
        log.error("HOOK ERROR: No data collected.")
        return

    output_path = os.path.join(config.site_dir, 'flat_index.json')
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    
    with open(output_path, 'w', encoding='utf-8') as f:
        json.dump(flat_index, f, indent=2, ensure_ascii=False)
    
    log.info(f"SUCCESS: Flat API index created with {len(flat_index)} entries.")