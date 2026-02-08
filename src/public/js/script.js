    const toggleBtn = document.getElementById('toggleRoadmap');
    const fontSelector = document.getElementById('fontSelector');
    const pageTitle = document.getElementById('pageTitle');

    toggleBtn.addEventListener('click', () => {
      document.body.classList.toggle('light');
      const isLight = document.body.classList.contains('light');
      toggleBtn.textContent = isLight ? 'Switch to Dark Mode' : 'Switch to Light Mode';
      toggleBtn.setAttribute('aria-pressed', isLight.toString());
    });

    fontSelector.addEventListener('change', () => {
      const font = fontSelector.value;
      document.body.style.fontFamily = font;
      pageTitle.style.fontFamily = font;
    });