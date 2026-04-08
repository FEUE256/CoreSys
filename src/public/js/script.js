// Set current year in all elements with class 'year'
document.querySelectorAll('.year').forEach(el => {
    el.textContent = new Date().getFullYear();
});

// Font selector logic
const fontSelector = document.getElementById('fontSelector');
fontSelector.addEventListener('change', () => {
    document.body.style.fontFamily = fontSelector.value;
});

// Optional: set initial font on page load
document.addEventListener('DOMContentLoaded', () => {
    document.body.style.fontFamily = fontSelector.value;
});