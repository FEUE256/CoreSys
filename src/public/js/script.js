
    const fontSelector = document.getElementById('fontSelector');
    const pageTitle = document.getElementById('pageTitle');

    fontSelector.addEventListener('change', () => {
      const font = fontSelector.value;
      document.body.style.fontFamily = font;
      pageTitle.style.fontFamily = font;
    });

  // Select all elements with class 'year'
  const yearElements = document.querySelectorAll('.year');

  // Get current year
  const currentYear = new Date().getFullYear();

  // Set the text of each element to the current year
  yearElements.forEach(el => {
    el.textContent = currentYear;
  });